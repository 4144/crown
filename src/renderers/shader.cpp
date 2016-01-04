/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "shader.h"
#include "config.h"
#include "filesystem.h"
#include "os.h"
#include "resource_manager.h"
#include "compile_options.h"
#include "temp_allocator.h"
#include "string_stream.h"
#include "sjson.h"
#include "map.h"
#include "shader_manager.h"
#include "device.h"

#if CROWN_DEBUG
#	define SHADERC_NAME "shaderc-debug-"
#else
#	define SHADERC_NAME "shaderc-development-"
#endif // CROWN_DEBUG
#if CROWN_ARCH_32BIT
#	define SHADERC_BITS "32"
#elif CROWN_ARCH_64BIT
#	define SHADERC_BITS "64"
#endif // CROWN_ARCH_32BIT
#if CROWN_PLATFORM_LINUX
#	define SHADERC_PATH "./" SHADERC_NAME "" SHADERC_BITS
#elif CROWN_PLATFORM_WINDOWS
#	define SHADERC_PATH SHADERC_NAME "" SHADERC_BITS ".exe"
#else
# 	define SHADERC_PATH ""
#endif // CROWN_PLATFORM_LINUX

namespace crown
{
namespace shader_resource
{
	int run_external_compiler(const char* infile, const char* outfile, const char* varying, const char* type, const char* platform, StringStream& output)
	{
		using namespace string_stream;

		TempAllocator512 ta;
		StringStream args(ta);
		args << " -f " << infile;
		args << " -o " << outfile;
		args << " --varyingdef " << varying;
		args << " --type " << type;
		args << " --platform " << platform;
		args << " --profile ";
#if CROWN_PLATFORM_LINUX
		args <<	"120";
#elif CROWN_PLATFORM_WINDOWS
		args << ((strcmp(type, "vertex") == 0) ? "vs_3_0" : "ps_3_0");
#endif

		return os::execute_process(SHADERC_PATH, c_str(args), output);
	}

	struct DepthTest
	{
		enum Enum
		{
			LESS,
			LEQUAL,
			EQUAL,
			GEQUAL,
			GREATER,
			NOTEQUAL,
			NEVER,
			ALWAYS,

			COUNT
		};
	};

	struct DepthTestInfo
	{
		const char* name;
		DepthTest::Enum value;
	};

	static DepthTestInfo _depth_test_map[] =
	{
		{ "less",     DepthTest::LESS     },
		{ "lequal",   DepthTest::LEQUAL   },
		{ "equal",    DepthTest::EQUAL    },
		{ "gequal",   DepthTest::GEQUAL   },
		{ "greater",  DepthTest::GREATER  },
		{ "notequal", DepthTest::NOTEQUAL },
		{ "never",    DepthTest::NEVER    },
		{ "always",   DepthTest::ALWAYS   }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_depth_test_map) == DepthTest::COUNT);

	static DepthTest::Enum name_to_depth_test(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_depth_test_map); ++i)
		{
			if (strcmp(name, _depth_test_map[i].name) == 0)
				return _depth_test_map[i].value;
		}

		return DepthTest::COUNT;
	}

	struct BlendFunction
	{
		enum Enum
		{
			ZERO,
			ONE,
			SRC_COLOR,
			INV_SRC_COLOR,
			SRC_ALPHA,
			INV_SRC_ALPHA,
			DST_ALPHA,
			INV_DST_ALPHA,
			DST_COLOR,
			INV_DST_COLOR,
			SRC_ALPHA_SAT,
			FACTOR,
			INV_FACTOR,

			COUNT
		};
	};

	struct BlendFunctionInfo
	{
		const char* name;
		BlendFunction::Enum value;
	};

	static BlendFunctionInfo _blend_function_map[] =
	{
		{ "zero",          BlendFunction::ZERO          },
		{ "one",           BlendFunction::ONE           },
		{ "src_color",     BlendFunction::SRC_COLOR     },
		{ "inv_src_color", BlendFunction::INV_SRC_COLOR },
		{ "src_alpha",     BlendFunction::SRC_ALPHA     },
		{ "inv_src_alpha", BlendFunction::INV_SRC_ALPHA },
		{ "dst_alpha",     BlendFunction::DST_ALPHA     },
		{ "inv_dst_alpha", BlendFunction::INV_DST_ALPHA },
		{ "dst_color",     BlendFunction::DST_COLOR     },
		{ "inv_dst_color", BlendFunction::INV_DST_COLOR },
		{ "src_alpha_sat", BlendFunction::SRC_ALPHA_SAT },
		{ "factor",        BlendFunction::FACTOR        },
		{ "inv_factor",    BlendFunction::INV_FACTOR    }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_blend_function_map) == BlendFunction::COUNT);

	static BlendFunction::Enum name_to_blend_function(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_blend_function_map); ++i)
		{
			if (strcmp(name, _blend_function_map[i].name) == 0)
				return _blend_function_map[i].value;
		}

		return BlendFunction::COUNT;
	}

	struct BlendEquation
	{
		enum Enum
		{
			ADD,
			SUB,
			REVSUB,
			MIN,
			MAX,

			COUNT
		};
	};

	struct BlendEquationInfo
	{
		const char* name;
		BlendEquation::Enum value;
	};

	static BlendEquationInfo _blend_equation_map[] =
	{
		{ "add",    BlendEquation::ADD    },
		{ "sub",    BlendEquation::SUB    },
		{ "revsub", BlendEquation::REVSUB },
		{ "min",    BlendEquation::MIN    },
		{ "max",    BlendEquation::MAX    }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_blend_equation_map) == BlendEquation::COUNT);

	static BlendEquation::Enum name_to_blend_equation(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_blend_equation_map); ++i)
		{
			if (strcmp(name, _blend_equation_map[i].name) == 0)
				return _blend_equation_map[i].value;
		}

		return BlendEquation::COUNT;
	}

	struct CullMode
	{
		enum Enum
		{
			CW,
			CCW,

			COUNT
		};
	};

	struct CullModeInfo
	{
		const char* name;
		CullMode::Enum value;
	};

	static CullModeInfo _cull_mode_map[] =
	{
		{ "cw",  CullMode::CW  },
		{ "ccw", CullMode::CCW }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_cull_mode_map) == CullMode::COUNT);

	static CullMode::Enum name_to_cull_mode(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_cull_mode_map); ++i)
		{
			if (strcmp(name, _cull_mode_map[i].name) == 0)
				return _cull_mode_map[i].value;
		}

		return CullMode::COUNT;
	}

	struct PrimitiveType
	{
		enum Enum
		{
			PT_TRISTRIP,
			PT_LINES,
			PT_LINESTRIP,
			PT_POINTS,

			COUNT
		};
	};

	struct PrimitiveTypeInfo
	{
		const char* name;
		PrimitiveType::Enum value;
	};

	static PrimitiveTypeInfo _primitive_type_map[] =
	{
		{ "pt_tristrip",  PrimitiveType::PT_TRISTRIP  },
		{ "pt_lines",     PrimitiveType::PT_LINES     },
		{ "pt_linestrip", PrimitiveType::PT_LINESTRIP },
		{ "pt_points",    PrimitiveType::PT_POINTS    }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(_primitive_type_map) == PrimitiveType::COUNT);

	static PrimitiveType::Enum name_to_primitive_type(const char* name)
	{
		for (uint32_t i = 0; i < CE_COUNTOF(_primitive_type_map); ++i)
		{
			if (strcmp(name, _primitive_type_map[i].name) == 0)
				return _primitive_type_map[i].value;
		}

		return PrimitiveType::COUNT;
	}

	struct RenderState
	{
		bool _rgb_write;
		bool _alpha_write;
		bool _depth_write;
		DepthTest::Enum _depth_test;
		BlendFunction::Enum _blend_src;
		BlendFunction::Enum _blend_dst;
		BlendEquation::Enum _blend_equation;
		CullMode::Enum _cull_mode;
		PrimitiveType::Enum _primitive_type;

		void reset()
		{
			_rgb_write = false;
			_alpha_write = false;
			_depth_write = false;
			_depth_test = DepthTest::COUNT;
			_blend_src = BlendFunction::COUNT;
			_blend_dst = BlendFunction::COUNT;
			_blend_equation = BlendEquation::COUNT;
			_cull_mode = CullMode::COUNT;
			_primitive_type = PrimitiveType::COUNT;
		}

		uint64_t encode() const
		{
			uint64_t state = 0;

			state |= (_rgb_write ? BGFX_STATE_RGB_WRITE : 0);
			state |= (_alpha_write ? BGFX_STATE_ALPHA_WRITE : 0);
			state |= (_depth_write ? BGFX_STATE_DEPTH_WRITE : 0);

			static uint64_t _bgfx_depth_test_map[] =
			{
				BGFX_STATE_DEPTH_TEST_LESS,     // DepthTest::LESS
				BGFX_STATE_DEPTH_TEST_LEQUAL,   // DepthTest::LEQUAL
				BGFX_STATE_DEPTH_TEST_EQUAL,    // DepthTest::EQUAL
				BGFX_STATE_DEPTH_TEST_GEQUAL,   // DepthTest::GEQUAL
				BGFX_STATE_DEPTH_TEST_GREATER,  // DepthTest::GREATER
				BGFX_STATE_DEPTH_TEST_NOTEQUAL, // DepthTest::NOTEQUAL
				BGFX_STATE_DEPTH_TEST_NEVER,    // DepthTest::NEVER
				BGFX_STATE_DEPTH_TEST_ALWAYS,   // DepthTest::ALWAYS
				0
			};

			static uint64_t _bgfx_blend_function_map[] =
			{
				BGFX_STATE_BLEND_ZERO,          // BlendFunction::ZERO
				BGFX_STATE_BLEND_ONE,           // BlendFunction::ONE
				BGFX_STATE_BLEND_SRC_COLOR,     // BlendFunction::SRC_COLOR
				BGFX_STATE_BLEND_INV_SRC_COLOR, // BlendFunction::INV_SRC_COLOR
				BGFX_STATE_BLEND_SRC_ALPHA,     // BlendFunction::SRC_ALPHA
				BGFX_STATE_BLEND_INV_SRC_ALPHA, // BlendFunction::INV_SRC_ALPHA
				BGFX_STATE_BLEND_DST_ALPHA,     // BlendFunction::DST_ALPHA
				BGFX_STATE_BLEND_INV_DST_ALPHA, // BlendFunction::INV_DST_ALPHA
				BGFX_STATE_BLEND_DST_COLOR,     // BlendFunction::DST_COLOR
				BGFX_STATE_BLEND_INV_DST_COLOR, // BlendFunction::INV_DST_COLOR
				BGFX_STATE_BLEND_SRC_ALPHA_SAT, // BlendFunction::SRC_ALPHA_SAT
				BGFX_STATE_BLEND_FACTOR,        // BlendFunction::FACTOR
				BGFX_STATE_BLEND_INV_FACTOR,    // BlendFunction::INV_FACTOR
				0
			};

			static uint64_t _bgfx_blend_equation_map[] =
			{
				BGFX_STATE_BLEND_EQUATION_ADD,    // BlendEquation::ADD
				BGFX_STATE_BLEND_EQUATION_SUB,    // BlendEquation::SUB
				BGFX_STATE_BLEND_EQUATION_REVSUB, // BlendEquation::REVSUB
				BGFX_STATE_BLEND_EQUATION_MIN,    // BlendEquation::MIN
				BGFX_STATE_BLEND_EQUATION_MAX,    // BlendEquation::MAX
				0
			};

			static uint64_t _bgfx_cull_mode_map[] =
			{
				BGFX_STATE_CULL_CW,  // CullMode::CW
				BGFX_STATE_CULL_CCW, // CullMode::CCW
				0
			};

			static uint64_t _bgfx_primitive_type_map[] =
			{
				BGFX_STATE_PT_TRISTRIP,  // PrimitiveType::PT_TRISTRIP
				BGFX_STATE_PT_LINES,     // PrimitiveType::PT_LINES
				BGFX_STATE_PT_LINESTRIP, // PrimitiveType::PT_LINESTRIP
				BGFX_STATE_PT_POINTS,    // PrimitiveType::PT_POINTS
				0
			};

			state |= _bgfx_depth_test_map[_depth_test];
			state |= BGFX_STATE_BLEND_FUNC(_bgfx_blend_function_map[_blend_src], _bgfx_blend_function_map[_blend_dst]);
			state |= BGFX_STATE_BLEND_EQUATION(_bgfx_blend_equation_map[_blend_equation]);
			state |= _bgfx_cull_mode_map[_cull_mode];
			state |= _bgfx_primitive_type_map[_primitive_type];

			return state;
		}
	};

	struct BgfxShader
	{
		BgfxShader()
			: _includes(default_allocator())
			, _code(default_allocator())
			, _vs_code(default_allocator())
			, _fs_code(default_allocator())
			, _varying(default_allocator())
			, _vs_input_output(default_allocator())
			, _fs_input_output(default_allocator())
		{
		}

		BgfxShader(Allocator& a)
			: _includes(a)
			, _code(a)
			, _vs_code(a)
			, _fs_code(a)
			, _varying(a)
			, _vs_input_output(a)
			, _fs_input_output(a)
		{
		}

		DynamicString _includes;
		DynamicString _code;
		DynamicString _vs_code;
		DynamicString _fs_code;
		DynamicString _varying;
		DynamicString _vs_input_output;
		DynamicString _fs_input_output;

		ALLOCATOR_AWARE;
	};

	struct ShaderPermutation
	{
		ShaderPermutation()
			: _bgfx_shader(default_allocator())
			, _render_state(default_allocator())
		{
		}

		ShaderPermutation(Allocator& a)
			: _bgfx_shader(a)
			, _render_state(a)
		{
		}

		DynamicString _bgfx_shader;
		DynamicString _render_state;
	};

	struct ShaderCompiler
	{
		CompileOptions& _opts;
		Map<DynamicString, RenderState> _render_states;
		Map<DynamicString, BgfxShader> _bgfx_shaders;
		Map<DynamicString, ShaderPermutation> _shaders;

		ShaderCompiler(CompileOptions& opts)
			: _opts(opts)
			, _render_states(default_allocator())
			, _bgfx_shaders(default_allocator())
			, _shaders(default_allocator())
		{
		}

		void parse(const char* path)
		{
			parse(_opts.read(path));
		}

		void parse(Buffer b)
		{
			TempAllocator4096 ta;
			JsonObject object(ta);
			sjson::parse(b, object);

			if (map::has(object, FixedString("include")))
			{
				JsonArray arr(ta);
				sjson::parse_array(object["include"], arr);

				for (uint32_t i = 0; i < array::size(arr); ++i)
				{
					DynamicString path(ta);
					sjson::parse_string(arr[i], path);
					parse(path.c_str());
				}
			}

			if (map::has(object, FixedString("render_states")))
				parse_render_states(object["render_states"]);

			if (map::has(object, FixedString("bgfx_shaders")))
				parse_bgfx_shaders(object["bgfx_shaders"]);

			if (map::has(object, FixedString("shaders")))
				parse_shaders(object["shaders"]);
		}

		void parse_render_states(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject render_states(ta);
			sjson::parse_object(json, render_states);

			const typename JsonObject::Node* begin = map::begin(render_states);
			const typename JsonObject::Node* end = map::end(render_states);
			for (; begin != end; ++begin)
			{
				JsonObject obj(ta);
				sjson::parse_object(begin->pair.second, obj);

				const bool rgb_write   = sjson::parse_bool(obj["rgb_write"]);
				const bool alpha_write = sjson::parse_bool(obj["alpha_write"]);
				const bool depth_write = sjson::parse_bool(obj["depth_write"]);

				DynamicString depth_test(ta);
				DynamicString blend_src(ta);
				DynamicString blend_dst(ta);
				DynamicString blend_equation(ta);
				DynamicString cull_mode(ta);
				DynamicString primitive_type(ta);
				if (map::has(obj, FixedString("depth_test")))
					sjson::parse_string(obj["depth_test"], depth_test);
				if (map::has(obj, FixedString("blend_src")))
					sjson::parse_string(obj["blend_src"], blend_src);
				if (map::has(obj, FixedString("blend_dst")))
					sjson::parse_string(obj["blend_dst"], blend_dst);
				if (map::has(obj, FixedString("blend_equation")))
					sjson::parse_string(obj["blend_equation"], blend_equation);
				if (map::has(obj, FixedString("cull_mode")))
					sjson::parse_string(obj["cull_mode"], cull_mode);
				if (map::has(obj, FixedString("primitive_type")))
					sjson::parse_string(obj["primitive_type"], primitive_type);

				DynamicString key(ta);
				key = begin->pair.first;

				RenderState rs;
				rs.reset();
				rs._rgb_write      = rgb_write;
				rs._alpha_write    = alpha_write;
				rs._depth_write    = depth_write;
				rs._depth_test     = name_to_depth_test(depth_test.c_str());
				rs._blend_src      = name_to_blend_function(blend_src.c_str());
				rs._blend_dst      = name_to_blend_function(blend_dst.c_str());
				rs._blend_equation = name_to_blend_equation(blend_equation.c_str());
				rs._cull_mode      = name_to_cull_mode(cull_mode.c_str());
				rs._primitive_type = name_to_primitive_type(primitive_type.c_str());

				map::set(_render_states, key, rs);
			}
		}

		void parse_bgfx_shaders(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject bgfx_shaders(ta);
			sjson::parse_object(json, bgfx_shaders);

			const typename JsonObject::Node* begin = map::begin(bgfx_shaders);
			const typename JsonObject::Node* end = map::end(bgfx_shaders);
			for (; begin != end; ++begin)
			{
				JsonObject shader(ta);
				sjson::parse_object(begin->pair.second, shader);

				BgfxShader bgfxshader(default_allocator());
				if (map::has(shader, FixedString("includes")))
					sjson::parse_string(shader["includes"], bgfxshader._includes);
				if (map::has(shader, FixedString("code")))
					sjson::parse_string(shader["code"], bgfxshader._code);
				if (map::has(shader, FixedString("vs_code")))
					sjson::parse_string(shader["vs_code"], bgfxshader._vs_code);
				if (map::has(shader, FixedString("fs_code")))
					sjson::parse_string(shader["fs_code"], bgfxshader._fs_code);
				if (map::has(shader, FixedString("varying")))
					sjson::parse_string(shader["varying"], bgfxshader._varying);
				if (map::has(shader, FixedString("vs_input_output")))
					sjson::parse_string(shader["vs_input_output"], bgfxshader._vs_input_output);
				if (map::has(shader, FixedString("fs_input_output")))
					sjson::parse_string(shader["fs_input_output"], bgfxshader._fs_input_output);

				DynamicString key(ta);
				key = begin->pair.first;

				map::set(_bgfx_shaders, key, bgfxshader);
			}
		}

		void parse_shaders(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject shaders(ta);
			sjson::parse_object(json, shaders);

			const typename JsonObject::Node* begin = map::begin(shaders);
			const typename JsonObject::Node* end = map::end(shaders);
			for (; begin != end; ++begin)
			{
				JsonObject obj(ta);
				sjson::parse_object(begin->pair.second, obj);

				ShaderPermutation shader(default_allocator());
				sjson::parse_string(obj["bgfx_shader"], shader._bgfx_shader);
				sjson::parse_string(obj["render_state"], shader._render_state);

				DynamicString key(ta);
				key = begin->pair.first;

				map::set(_shaders, key, shader);
			}
		}

		void compile()
		{
			_opts.write(SHADER_VERSION);
			_opts.write(map::size(_shaders));

			const typename Map<DynamicString, ShaderPermutation>::Node* begin = map::begin(_shaders);
			const typename Map<DynamicString, ShaderPermutation>::Node* end = map::end(_shaders);
			for (; begin != end; ++begin)
			{
				const ShaderPermutation& sp = begin->pair.second;
				const StringId32 shader_name = begin->pair.first.to_string_id();
				const char* bgfx_shader = sp._bgfx_shader.c_str();
				const char* render_state = sp._render_state.c_str();

				RESOURCE_COMPILER_ASSERT(map::has(_bgfx_shaders, sp._bgfx_shader)
					, _opts
					, "Unknown bgfx shader"
					);
				RESOURCE_COMPILER_ASSERT(map::has(_render_states, sp._render_state)
					, _opts
					, "Unknown render state"
					);
				const RenderState& rs = _render_states[render_state];

				_opts.write(shader_name._id); // Shader name
				_opts.write(rs.encode());     // Render state
				compile(bgfx_shader);         // Shader code
			}
		}

		void compile(const char* bgfx_shader)
		{
			const BgfxShader& shader = _bgfx_shaders[bgfx_shader];

			DynamicString included_code(default_allocator());
			if (!(shader._includes == ""))
			{
				const BgfxShader& included = _bgfx_shaders[shader._includes.c_str()];
				included_code = included._code;
			}

			DynamicString vs_code(default_allocator());
			DynamicString fs_code(default_allocator());
			vs_code += shader._vs_input_output;
			vs_code += included_code;
			vs_code += shader._code;
			vs_code += shader._vs_code;
			fs_code += shader._fs_input_output;
			fs_code += included_code;
			fs_code += shader._code;
			fs_code += shader._fs_code;

			DynamicString vs_code_path(default_allocator());
			DynamicString fs_code_path(default_allocator());
			DynamicString varying_def_path(default_allocator());
			DynamicString tmpvs_path(default_allocator());
			DynamicString tmpfs_path(default_allocator());

			_opts.get_absolute_path("vs_code.tmp", vs_code_path);
			_opts.get_absolute_path("fs_code.tmp", fs_code_path);
			_opts.get_absolute_path("varying.tmp", varying_def_path);
			_opts.get_absolute_path("tmpvs", tmpvs_path);
			_opts.get_absolute_path("tmpfs", tmpfs_path);

			File* vs_file = _opts._fs.open(vs_code_path.c_str(), FileOpenMode::WRITE);
			vs_file->write(vs_code.c_str(), vs_code.length());
			_opts._fs.close(*vs_file);

			File* fs_file = _opts._fs.open(fs_code_path.c_str(), FileOpenMode::WRITE);
			fs_file->write(fs_code.c_str(), fs_code.length());
			_opts._fs.close(*fs_file);

			File* varying_file = _opts._fs.open(varying_def_path.c_str(), FileOpenMode::WRITE);
			varying_file->write(shader._varying.c_str(), shader._varying.length());
			_opts._fs.close(*varying_file);

			TempAllocator4096 ta;
			StringStream output(ta);
			using namespace string_stream;

			int exitcode = run_external_compiler(vs_code_path.c_str()
				, tmpvs_path.c_str()
				, varying_def_path.c_str()
				, "vertex"
				, _opts.platform()
				, output
				);
			RESOURCE_COMPILER_ASSERT(exitcode == 0
				, _opts
				, "Failed to compile vertex shader:\n%s"
				, c_str(output)
				);

			array::clear(output);
			exitcode = run_external_compiler(fs_code_path.c_str()
				, tmpfs_path.c_str()
				, varying_def_path.c_str()
				, "fragment"
				, _opts.platform()
				, output
				);
			if (exitcode)
			{
				_opts.delete_file(tmpvs_path.c_str());
				RESOURCE_COMPILER_ASSERT(false
					, _opts
					, "Failed to compile fragment shader:\n%s"
					, c_str(output)
					);
			}

			_opts.delete_file(vs_code_path.c_str());
			_opts.delete_file(fs_code_path.c_str());
			_opts.delete_file(varying_def_path.c_str());

			Buffer tmpvs = _opts.read(tmpvs_path.c_str());
			Buffer tmpfs = _opts.read(tmpfs_path.c_str());

			_opts.delete_file(tmpvs_path.c_str());
			_opts.delete_file(tmpfs_path.c_str());

			// Write
			_opts.write(uint32_t(array::size(tmpvs)));
			_opts.write(array::begin(tmpvs), array::size(tmpvs));
			_opts.write(uint32_t(array::size(tmpfs)));
			_opts.write(array::begin(tmpfs), array::size(tmpfs));
		}
	};

	void compile(const char* path, CompileOptions& opts)
	{
		ShaderCompiler sc(opts);
		sc.parse(path);
		sc.compile();
	}

	void* load(File& file, Allocator& a)
	{
		return device()->shader_manager()->load(file, a);
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		device()->shader_manager()->online(id, rm);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		device()->shader_manager()->offline(id, rm);
	}

	void unload(Allocator& a, void* res)
	{
		device()->shader_manager()->unload(a, res);
	}
} // namespace shader_resource
} // namespace crown
