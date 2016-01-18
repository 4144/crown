/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sound_resource.h"
#include "dynamic_string.h"
#include "filesystem.h"
#include "compile_options.h"
#include "sjson.h"
#include "map.h"

namespace crown
{
namespace sound_resource
{
	struct WAVHeader
	{
		char    riff[4];         // Should contain 'RIFF'
		int32_t chunk_size;      // Not Needed
		char    wave[4];         // Should contain 'WAVE'
		char    fmt[4];          // Should contain 'fmt '
		int32_t fmt_size;        // Size of format chunk
		int16_t fmt_tag;         // Identifies way data is stored, 1 means no compression
		int16_t fmt_channels;    // Channel, 1 means mono, 2 means stereo
		int32_t fmt_sample_rate; // Samples per second
		int32_t fmt_avarage;     // Avarage bytes per sample
		int16_t fmt_block_align; // Block alignment
		int16_t fmt_bits_ps;     // Number of bits per sample
		char    data[4];         // Should contain 'data'
		int32_t data_size;       // Data dimension
	};

	void compile(const char* path, CompileOptions& opts)
	{
		Buffer buf = opts.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		DynamicString name(ta);
		sjson::parse_string(object["source"], name);

		Buffer sound = opts.read(name.c_str());
		const WAVHeader* wav = (const WAVHeader*)array::begin(sound);
		const char* wavdata = (const char*)&wav[1];

		// Write
		SoundResource sr;
		sr.version      = SOUND_VERSION;
		sr.size         = wav->data_size;
		sr.sample_rate  = wav->fmt_sample_rate;
		sr.avg_bytes_ps = wav->fmt_avarage;
		sr.channels     = wav->fmt_channels;
		sr.block_size   = wav->fmt_block_align;
		sr.bits_ps      = wav->fmt_bits_ps;
		sr.sound_type   = SoundType::WAV;

		opts.write(sr.version);
		opts.write(sr.size);
		opts.write(sr.sample_rate);
		opts.write(sr.avg_bytes_ps);
		opts.write(sr.channels);
		opts.write(sr.block_size);
		opts.write(sr.bits_ps);
		opts.write(sr.sound_type);

		opts.write(wavdata, wav->data_size);
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		CE_ASSERT(*(uint32_t*)res == SOUND_VERSION, "Wrong version");
		return res;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	const char* data(const SoundResource* sr)
	{
		return (char*)&sr[1];
	}
} // namespace sound_resource
} // namespace crown
