/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_SOUND_OPENSLES

#include "sound_world.h"
#include "id_array.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "sound_resource.h"
#include "temp_allocator.h"
#include "queue.h"
#include "audio.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace crown
{

#if defined(CROWN_DEBUG)
	static const char* sles_error_to_string(SLresult result)
	{
		switch (result)
		{
			case SL_RESULT_SUCCESS: return "SL_RESULT_SUCCESS";
			case SL_RESULT_PARAMETER_INVALID: return "SL_RESULT_PARAMETER_INVALID";
			case SL_RESULT_MEMORY_FAILURE: return "SL_RESULT_MEMORY_FAILURE";
			case SL_RESULT_FEATURE_UNSUPPORTED: return "SL_RESULT_FEATURE_UNSUPPORTED";
			case SL_RESULT_RESOURCE_ERROR: return "SL_RESULT_RESOURCE_ERROR";
			case SL_RESULT_IO_ERROR: return "SL_RESULT_IO_ERROR";
			case SL_RESULT_PRECONDITIONS_VIOLATED: return "SL_RESULT_PRECONDITIONS_VIOLATED";
			case SL_RESULT_CONTENT_CORRUPTED: return "SL_RESULT_CONTENT_CORRUPTED";
			case SL_RESULT_CONTENT_UNSUPPORTED: return "SL_RESULT_CONTENT_UNSUPPORTED";
			case SL_RESULT_CONTENT_NOT_FOUND: return "SL_RESULT_CONTENT_NOT_FOUND";
			case SL_RESULT_PERMISSION_DENIED: return "SL_RESULT_PERMISSION_DENIED";
			case SL_RESULT_BUFFER_INSUFFICIENT: return "SL_RESULT_BUFFER_INSUFFICIENT";
			default: return "UNKNOWN_SL_ERROR";
		}
	}

	#define SL_CHECK(function)\
		do { SLresult result = function;\
				CE_ASSERT(result == SL_RESULT_SUCCESS, "OpenSL|ES error: %s", sles_error_to_string(result)); } while (0)
#else
	#define SL_CHECK(function) function;
#endif

namespace audio_globals
{
	static SLObjectItf s_sl_engine;
	static SLEngineItf s_sl_engine_itf;
	static SLObjectItf s_sl_output_mix;

	void init()
	{
		const SLInterfaceID ids[] = {SL_IID_ENGINE};
		const SLboolean reqs[] = {SL_BOOLEAN_TRUE};
		const SLEngineOption opts[] = { (SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE };

		// Create OpenSL engine
		SL_CHECK(slCreateEngine(&s_sl_engine, 1, opts, 1, ids, reqs));
		SL_CHECK((*s_sl_engine)->Realize(s_sl_engine, SL_BOOLEAN_FALSE));

		// Obtain OpenSL engine interface
		SL_CHECK((*s_sl_engine)->GetInterface(s_sl_engine, SL_IID_ENGINE, &s_sl_engine_itf));

		// Create global output mix
		const SLInterfaceID ids1[] = {SL_IID_VOLUME};
		const SLboolean reqs1[] = {SL_BOOLEAN_FALSE};

		SL_CHECK((*s_sl_engine_itf)->CreateOutputMix(s_sl_engine_itf, &s_sl_output_mix, 1, ids1, reqs1));
		SL_CHECK((*s_sl_output_mix)->Realize(s_sl_output_mix, SL_BOOLEAN_FALSE));
	}

	void shutdown()
	{
		(*s_sl_output_mix)->Destroy(s_sl_output_mix);
		(*s_sl_engine)->Destroy(s_sl_engine);
	}
} // namespace audio_globals

namespace sles_sound_world
{
	// Queue of instances to stop at next update()
	static Queue<SoundInstanceId>* s_stop_queue = NULL;

	void init()
	{
		s_stop_queue = CE_NEW(default_allocator(), Queue<SoundInstanceId>)(default_allocator());
	}

	void shutdown()
	{
		CE_DELETE(default_allocator(), s_stop_queue);
	}

	static void player_callback(SLPlayItf caller, void* context, SLuint32 event)
	{
		SoundInstanceId id;
		id.decode((uint32_t) context);
		queue::push_back(*s_stop_queue, id);
	}

	static SLmillibel gain_to_attenuation(SLVolumeItf vol_itf, float volume)
	{
		SLmillibel volume_mb;
		if (volume <= 0.02f) return SL_MILLIBEL_MIN;
		else if (volume >= 1.0f)
		{
			(*vol_itf)->GetMaxVolumeLevel(vol_itf, &volume_mb);
			return volume_mb;
		}

		volume_mb = M_LN2 / log(1.0f / (1.0f - volume)) * -1000.0f;
		if (volume_mb > 0) volume_mb = SL_MILLIBEL_MIN;
		return volume_mb;
	}
}

struct SoundInstance
{
	void create(SLEngineItf engine, SLObjectItf output_mix, SoundInstanceId id, SoundResource* sr)
	{
		using namespace sound_resource;

		_resource = sr;
		_finished = false;
		_id = id;

		// Configures buffer queue
		SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

		// Configures audio format
		SLDataFormat_PCM format_pcm;
		format_pcm.formatType = SL_DATAFORMAT_PCM;

		// Sets channels
		switch (channels(sr))
		{
			case 1:
			{
				format_pcm.numChannels = 1;
				format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
				break;
			}
			case 2:
			{
				format_pcm.numChannels = 2;
				format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
				break;
			}
			default:
			{
				CE_FATAL("Oops, wrong number of channels");
				break;
			}
		}

		// Sets sample rate
		switch (sample_rate(sr))
		{
			case 8000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_8; break;
			case 11025: format_pcm.samplesPerSec = SL_SAMPLINGRATE_11_025; break;
			case 16000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_16; break;
			case 22050: format_pcm.samplesPerSec = SL_SAMPLINGRATE_22_05; break;
			case 24000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_24; break;
			case 32000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_32; break;
			case 44100: format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1; break;
			case 48000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_48; break;
			case 64000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_64; break;
			case 88200: format_pcm.samplesPerSec = SL_SAMPLINGRATE_88_2; break;
			case 96000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_96; break;
			case 192000: format_pcm.samplesPerSec = SL_SAMPLINGRATE_192; break;
			default: CE_FATAL("Oops, sample rate not supported"); break;
		}

		format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
		format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
		format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

		// Configures audio source
		SLDataSource audio_source;
		audio_source.pLocator = &buffer_queue;
		audio_source.pFormat = &format_pcm;

		// Configures audio output mix
		SLDataLocator_OutputMix out_mix;
		out_mix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		out_mix.outputMix = output_mix;

		// Configures audio sink
		SLDataSink audio_sink;
		audio_sink.pLocator = &out_mix;
		audio_sink.pFormat = NULL;

		// Creates sound player
		const SLInterfaceID ids[] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
		const SLboolean reqs[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

		SL_CHECK((*engine)->CreateAudioPlayer(engine, &_player, &audio_source, &audio_sink, 3, ids, reqs));
		SL_CHECK((*_player)->Realize(_player, SL_BOOLEAN_FALSE));

		SL_CHECK((*_player)->GetInterface(_player, SL_IID_BUFFERQUEUE, &_player_bufferqueue));

		//(*_player_bufferqueue)->RegisterCallback(_player_bufferqueue, SoundInstance::buffer_callback, this);
		(*play_itf())->SetCallbackEventsMask(play_itf(), SL_PLAYEVENT_HEADATEND);
		(*play_itf())->RegisterCallback(play_itf(), sles_sound_world::player_callback, (void*) id.encode());

		// Manage simple sound or stream
		// m_streaming = sound_type(sr) == SoundType::OGG;

		// if (m_streaming)
		// {
		// 	m_decoder.init((char*)data(sr), size(sr));

		// 	m_decoder.stream();
		// 	(*_player_bufferqueue)->Enqueue(_player_bufferqueue, m_decoder.data(), m_decoder.size());
		// }
		// else
		{
			(*_player_bufferqueue)->Enqueue(_player_bufferqueue, data(sr), size(sr));
		}
	}

	void destroy()
	{
		// if (m_streaming)
		// {
		// 	m_decoder.shutdown();
		// }
		stop();
		(*_player_bufferqueue)->Clear(_player_bufferqueue);
		(*_player)->AbortAsyncOperation(_player);
		(*_player)->Destroy(_player);
	}

	void reload(SoundResource* new_sr)
	{
	}

	void play(bool loop, float volume)
	{
		set_volume(volume);
		SL_CHECK((*play_itf())->SetPlayState(play_itf(), SL_PLAYSTATE_PLAYING));
	}

	void pause()
	{
		SL_CHECK((*play_itf())->SetPlayState(play_itf(), SL_PLAYSTATE_PAUSED));
	}

	void resume()
	{
		SL_CHECK((*play_itf())->SetPlayState(play_itf(), SL_PLAYSTATE_PLAYING));
	}

	void stop()
	{
		SL_CHECK((*play_itf())->SetPlayState(play_itf(), SL_PLAYSTATE_STOPPED));
	}

	bool is_playing()
	{
		SLuint32 state;
		SL_CHECK((*play_itf())->GetPlayState(play_itf(), &state));
		return state == SL_PLAYSTATE_PLAYING;
	}

	bool finished()
	{
		return _finished;
	}

	void set_volume(float volume)
	{
		SLVolumeItf vol;
		SL_CHECK((*_player)->GetInterface(_player, SL_IID_VOLUME, &vol));
		SL_CHECK((*vol)->SetVolumeLevel(vol, sles_sound_world::gain_to_attenuation(vol, volume)));
	}

	void set_range(float range)
	{
	}

	void set_position(const Vector3& pos)
	{
	}

	static void buffer_callback(SLAndroidSimpleBufferQueueItf caller, void* context)
	{
		SoundInstance* s = (SoundInstance*) context;

		// if (s->is_playing())
		// {
		// 	s->_processed_buffers++;

		// 	if (s->m_decoder.stream())
		// 	{
		// 		(*s->_player_bufferqueue)->Enqueue(s->_player_bufferqueue, s->m_decoder.data(), s->m_decoder.size());
		// 	}
		// 	else if (s->m_looping)
		// 	{
		// 		s->m_decoder.rewind();
		// 		s->m_decoder.stream();
		// 		(*s->_player_bufferqueue)->Enqueue(s->_player_bufferqueue, s->m_decoder.data(), s->m_decoder.size());
		// 	}
		// 	else
		// 	{
		// 		s->pause();
		// 	}
		// }
	}

	SoundResource* resource()
	{
		return _resource;
	}

	SLPlayItf play_itf()
	{
		SLPlayItf play;
		SL_CHECK((*_player)->GetInterface(_player, SL_IID_PLAY, &play));
		return play;
	}

public:

	SoundInstanceId _id;
	SoundResource* _resource;

	SLObjectItf _player;
	SLAndroidSimpleBufferQueueItf _player_bufferqueue;

	uint32_t _processed_buffers;
	bool _finished;
};

class SLESSoundWorld : public SoundWorld
{
public:

	SLESSoundWorld()
	{
		sles_sound_world::init();
	}

	virtual ~SLESSoundWorld()
	{
		sles_sound_world::shutdown();
	}

	virtual SoundInstanceId play(SoundResource* sr, bool loop, float volume, const Vector3& /*pos*/)
	{
		SoundInstance dummy;
		SoundInstanceId id = id_array::create(_playing_sounds, dummy);

		SoundInstance& instance = id_array::get(_playing_sounds, id);
		instance.create(audio_globals::s_sl_engine_itf, audio_globals::s_sl_output_mix, id, sr);
		instance.play(loop, volume);

		return id;
	}

	virtual void stop(SoundInstanceId id)
	{
		id_array::get(_playing_sounds, id).destroy();
		id_array::destroy(_playing_sounds, id);
	}

	virtual bool is_playing(SoundInstanceId id)
	{
		return id_array::has(_playing_sounds, id) && id_array::get(_playing_sounds, id).is_playing();
	}

	virtual void stop_all()
	{
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			_playing_sounds[i].stop();
		}
	}

	virtual void pause_all()
	{
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			_playing_sounds[i].pause();
		}
	}

	virtual void resume_all()
	{
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			_playing_sounds[i].resume();
		}
	}

	virtual void set_sound_positions(uint32_t num, const SoundInstanceId* ids, const Vector3* positions)
	{
		for (uint32_t i = 0; i < num; i++)
		{
			id_array::get(_playing_sounds, ids[i]).set_position(positions[i]);
		}
	}

	virtual void set_sound_ranges(uint32_t num, const SoundInstanceId* ids, const float* ranges)
	{
		for (uint32_t i = 0; i < num; i++)
		{
			id_array::get(_playing_sounds, ids[i]).set_range(ranges[i]);
		}
	}

	virtual void set_sound_volumes(uint32_t num, const SoundInstanceId* ids, const float* volumes)
	{
		for (uint32_t i = 0; i < num; i++)
		{
			id_array::get(_playing_sounds, ids[i]).set_volume(volumes[i]);
		}
	}

	virtual void reload_sounds(SoundResource* old_sr, SoundResource* new_sr)
	{
		for (uint32_t i = 0; i < id_array::size(_playing_sounds); i++)
		{
			if (_playing_sounds[i].resource() == old_sr)
			{
				_playing_sounds[i].reload(new_sr);
			}
		}
	}

	virtual void set_listener_pose(const Matrix4x4& pose)
	{
		_listener_pose = pose;
	}

	virtual void update()
	{
		const uint32_t num_to_stop = queue::size(*sles_sound_world::s_stop_queue);
		for (uint32_t i = 0; i < num_to_stop; i++)
		{
			const SoundInstanceId id = queue::front(*sles_sound_world::s_stop_queue);
			queue::pop_front(*sles_sound_world::s_stop_queue);

			if (!id_array::has(_playing_sounds, id)) continue;
			stop(id);
		}
	}

private:

	IdArray<CE_MAX_SOUND_INSTANCES, SoundInstance> _playing_sounds;
	Matrix4x4 _listener_pose;
};

SoundWorld* SoundWorld::create(Allocator& a)
{
	return CE_NEW(a, SLESSoundWorld)();
}

void SoundWorld::destroy(Allocator& a, SoundWorld* sw)
{
	CE_DELETE(a, sw);
}

} // namespace crown

#endif // CROWN_SOUND_OPENSLES
