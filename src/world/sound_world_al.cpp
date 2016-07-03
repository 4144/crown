/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_SOUND_OPENAL

#include "array.h"
#include "audio.h"
#include "log.h"
#include "matrix4x4.h"
#include "sound_resource.h"
#include "sound_world.h"
#include "temp_allocator.h"
#include "vector3.h"
#include <AL/al.h>
#include <AL/alc.h>

namespace crown
{
#if CROWN_DEBUG
	static const char* al_error_to_string(ALenum error)
	{
		switch (error)
		{
			case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
			case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
			case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
			case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
			default: return "UNKNOWN_AL_ERROR";
		}
	}

	#define AL_CHECK(function)\
		function;\
		do { ALenum error; CE_ASSERT((error = alGetError()) == AL_NO_ERROR,\
				"OpenAL error: %s", al_error_to_string(error)); } while (0)
#else
	#define AL_CHECK(function) function
#endif // CROWN_DEBUG

/// Global audio-related functions
namespace audio_globals
{
	static ALCdevice* s_al_device;
	static ALCcontext* s_al_context;

	void init()
	{
		s_al_device = alcOpenDevice(NULL);
		CE_ASSERT(s_al_device, "Cannot open OpenAL audio device");

		s_al_context = alcCreateContext(s_al_device, NULL);
		CE_ASSERT(s_al_context, "Cannot create OpenAL context");

		AL_CHECK(alcMakeContextCurrent(s_al_context));

		logd("OpenAL Vendor   : %s", alGetString(AL_VENDOR));
		logd("OpenAL Version  : %s", alGetString(AL_VERSION));
		logd("OpenAL Renderer : %s", alGetString(AL_RENDERER));

		AL_CHECK(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED));
		AL_CHECK(alDopplerFactor(1.0f));
		AL_CHECK(alDopplerVelocity(343.0f));
	}

	void shutdown()
	{
		alcDestroyContext(s_al_context);
	    alcCloseDevice(s_al_device);
	}
}

struct SoundInstance
{
	void create(const SoundResource& sr, const Vector3& pos, f32 range)
	{
		using namespace sound_resource;

		AL_CHECK(alGenSources(1, &_source));
		CE_ASSERT(alIsSource(_source), "alGenSources: error");

		AL_CHECK(alSourcef(_source, AL_REFERENCE_DISTANCE, 0.01f));
		AL_CHECK(alSourcef(_source, AL_MAX_DISTANCE, range));
		AL_CHECK(alSourcef(_source, AL_PITCH, 1.0f));

		// Generates AL buffers
		AL_CHECK(alGenBuffers(1, &_buffer));
		CE_ASSERT(alIsBuffer(_buffer), "alGenBuffers: error");

		ALenum fmt = AL_INVALID_ENUM;
		switch (sr.bits_ps)
		{
			case  8: fmt = sr.channels > 1 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8; break;
			case 16: fmt = sr.channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16; break;
			default: CE_FATAL("Number of bits per sample not supported."); break;
		}
		AL_CHECK(alBufferData(_buffer, fmt, data(&sr), sr.size, sr.sample_rate));

		_resource = &sr;
		set_position(pos);
	}

	void destroy()
	{
		stop();
		AL_CHECK(alSourcei(_source, AL_BUFFER, 0));
		AL_CHECK(alDeleteBuffers(1, &_buffer));
		AL_CHECK(alDeleteSources(1, &_source));
	}

	void reload(const SoundResource& new_sr)
	{
		destroy();
		create(new_sr, position(), range());
	}

	void play(bool loop, f32 volume)
	{
		set_volume(volume);
		AL_CHECK(alSourcei(_source, AL_LOOPING, (loop ? AL_TRUE : AL_FALSE)));
		AL_CHECK(alSourceQueueBuffers(_source, 1, &_buffer));
		AL_CHECK(alSourcePlay(_source));
	}

	void pause()
	{
		AL_CHECK(alSourcePause(_source));
	}

	void resume()
	{
		AL_CHECK(alSourcePlay(_source));
	}

	void stop()
	{
		AL_CHECK(alSourceStop(_source));
		AL_CHECK(alSourceRewind(_source)); // Workaround
		ALint processed;
		AL_CHECK(alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed));

		if (processed > 0)
		{
			ALuint removed;
			AL_CHECK(alSourceUnqueueBuffers(_source, 1, &removed));
		}
	}

	bool is_playing()
	{
		ALint state;
		AL_CHECK(alGetSourcei(_source, AL_SOURCE_STATE, &state));
		return state == AL_PLAYING;
	}

	bool finished()
	{
		ALint state;
		AL_CHECK(alGetSourcei(_source, AL_SOURCE_STATE, &state));
		return (state != AL_PLAYING && state != AL_PAUSED);
	}

	Vector3 position()
	{
		ALfloat pos[3];
		AL_CHECK(alGetSourcefv(_source, AL_POSITION, pos));
		return vector3(pos[0], pos[1], pos[2]);
	}

	float range()
	{
		ALfloat range;
		AL_CHECK(alGetSourcefv(_source, AL_MAX_DISTANCE, &range));
		return range;
	}

	void set_position(const Vector3& pos)
	{
		AL_CHECK(alSourcefv(_source, AL_POSITION, to_float_ptr(pos)));
	}

	void set_range(f32 range)
	{
		AL_CHECK(alSourcef(_source, AL_MAX_DISTANCE, range));
	}

	void set_volume(f32 volume)
	{
		AL_CHECK(alSourcef(_source, AL_GAIN, volume));
	}

	const SoundResource* resource()
	{
		return _resource;
	}

public:

	const SoundResource* _resource;
	SoundInstanceId _id;
	ALuint _buffer;
	ALuint _source;
};

#define MAX_OBJECTS       1024
#define INDEX_MASK        0xffff
#define NEW_OBJECT_ID_ADD 0x10000

class ALSoundWorld : public SoundWorld
{
	struct Index
	{
		SoundInstanceId id;
		u16 index;
		u16 next;
	};

	u32 _num_objects;
	SoundInstance _playing_sounds[MAX_OBJECTS];
	Index _indices[MAX_OBJECTS];
	u16 _freelist_enqueue;
	u16 _freelist_dequeue;
	Matrix4x4 _listener_pose;

	bool has(SoundInstanceId id)
	{
		Index& in = _indices[id & INDEX_MASK];
		return in.id == id && in.index != UINT16_MAX;
	}

	SoundInstance& lookup(SoundInstanceId id)
	{
		return _playing_sounds[_indices[id & INDEX_MASK].index];
	}

	SoundInstanceId add()
	{
		Index& in = _indices[_freelist_dequeue];
		_freelist_dequeue = in.next;
		in.id += NEW_OBJECT_ID_ADD;
		in.index = _num_objects++;
		SoundInstance& o = _playing_sounds[in.index];
		o._id = in.id;
		return o._id;
	}

	void remove(SoundInstanceId id)
	{
		Index& in = _indices[id & INDEX_MASK];

		SoundInstance& o = _playing_sounds[in.index];
		o = _playing_sounds[--_num_objects];
		_indices[o._id & INDEX_MASK].index = in.index;

		in.index = UINT16_MAX;
		_indices[_freelist_enqueue].next = id & INDEX_MASK;
		_freelist_enqueue = id & INDEX_MASK;
	}

public:

	ALSoundWorld()
	{
		_num_objects = 0;
		for (u32 i = 0; i < MAX_OBJECTS; ++i)
		{
			_indices[i].id = i;
			_indices[i].next = i + 1;
		}
		_freelist_dequeue = 0;
		_freelist_enqueue = MAX_OBJECTS - 1;

		set_listener_pose(MATRIX4X4_IDENTITY);
	}

	virtual ~ALSoundWorld()
	{
	}

	virtual SoundInstanceId play(const SoundResource& sr, bool loop, f32 volume, f32 range, const Vector3& pos)
	{
		SoundInstanceId id = add();
		SoundInstance& si = lookup(id);
		si.create(sr, pos, range);
		si.play(loop, volume);
		return id;
	}

	virtual void stop(SoundInstanceId id)
	{
		SoundInstance& si = lookup(id);
		si.destroy();
		remove(id);
	}

	virtual bool is_playing(SoundInstanceId id)
	{
		return has(id) && lookup(id).is_playing();
	}

	virtual void stop_all()
	{
		for (u32 i = 0; i < _num_objects; ++i)
		{
			_playing_sounds[i].stop();
		}
	}

	virtual void pause_all()
	{
		for (u32 i = 0; i < _num_objects; ++i)
		{
			_playing_sounds[i].pause();
		}
	}

	virtual void resume_all()
	{
		for (u32 i = 0; i < _num_objects; ++i)
		{
			_playing_sounds[i].resume();
		}
	}

	virtual void set_sound_positions(u32 num, const SoundInstanceId* ids, const Vector3* positions)
	{
		for (u32 i = 0; i < num; ++i)
		{
			lookup(ids[i]).set_position(positions[i]);
		}
	}

	virtual void set_sound_ranges(u32 num, const SoundInstanceId* ids, const f32* ranges)
	{
		for (u32 i = 0; i < num; ++i)
		{
			lookup(ids[i]).set_range(ranges[i]);
		}
	}

	virtual void set_sound_volumes(u32 num, const SoundInstanceId* ids, const f32* volumes)
	{
		for (u32 i = 0; i < num; i++)
		{
			lookup(ids[i]).set_volume(volumes[i]);
		}
	}

	virtual void reload_sounds(const SoundResource& old_sr, const SoundResource& new_sr)
	{
		for (u32 i = 0; i < _num_objects; ++i)
		{
			if (_playing_sounds[i].resource() == &old_sr)
			{
				_playing_sounds[i].reload(new_sr);
			}
		}
	}

	virtual void set_listener_pose(const Matrix4x4& pose)
	{
		const Vector3 pos = translation(pose);
		const Vector3 up = y(pose);
		const Vector3 at = z(pose);

		AL_CHECK(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));
		//AL_CHECK(alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z));

		const ALfloat orientation[] = { up.x, up.y, up.z, at.x, at.y, at.z };
		AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
		_listener_pose = pose;
	}

	virtual void update()
	{
		TempAllocator256 alloc;
		Array<SoundInstanceId> to_delete(alloc);

		// Check what sounds finished playing
		for (u32 i = 0; i < _num_objects; ++i)
		{
			SoundInstance& instance = _playing_sounds[i];
			if (instance.finished())
			{
				array::push_back(to_delete, instance._id);
			}
		}

		// Destroy instances which finished playing
		for (u32 i = 0; i < array::size(to_delete); ++i)
		{
			stop(to_delete[i]);
		}
	}
};

namespace sound_world
{
	SoundWorld* create(Allocator& a)
	{
		return CE_NEW(a, ALSoundWorld)();
	}

	void destroy(Allocator& a, SoundWorld* sw)
	{
		CE_DELETE(a, sw);
	}
} // namespace sound_world

} // namespace crown

#endif // CROWN_SOUND_OPENAL
