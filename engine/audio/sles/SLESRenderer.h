/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "SoundResource.h"
#include "OggDecoder.h"
#include "Vec3.h"
#include "Log.h"

namespace crown
{

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
	default: return "SL_RESULT_UNKNOWN";

	}
}

//-----------------------------------------------------------------------------
void check_sles_errors(SLresult result)
{
	CE_ASSERT(result == SL_RESULT_SUCCESS, "SL_ERROR_CODE: %s", sles_error_to_string(result));
}

//-----------------------------------------------------------------------------
struct Sound
{
					Sound();

	void 			create(SLEngineItf engine, SLObjectItf out_mix_obj, SoundResource* resource);
	void 			update();
	void 			destroy();
	void 			play();
	void 			pause();
	void 			loop(bool loop);
	void 			set_min_distance(const float min_distance);
	void 			set_max_distance( const float max_distance);
	void 			set_position(const Vec3& pos);
	void 			set_velocity(const Vec3& vel);
	void 			set_direction(const Vec3& dir);
	void 			set_pitch(const float pitch);
	void 			set_gain(const float gain);
	void 			set_rolloff(const float rolloff);
	float 			min_distance() const;
	float 			max_distance() const;
	Vec3 			position() const;
	Vec3 			velocity() const;
	Vec3 			direction() const;
	float 			pitch() const;
	float 			gain() const;
	float 			rolloff() const;
	bool 			is_playing() const;
	int32_t 		queued_buffers();
	int32_t 		processed_buffers();

	static void 	buffer_callback(SLAndroidSimpleBufferQueueItf caller, void* sound);

private:

	void create_stream();

	void update_stream();

public:

	SoundResource*					m_res;

	SLEngineItf						m_engine;
	SLObjectItf 					m_out_mix_obj;
	SLObjectItf 					m_player_obj;
	SLPlayItf 						m_player_play;
	SLAndroidSimpleBufferQueueItf 	m_player_bufferqueue;
	SLSeekItf 						m_player_seek;
	SLVolumeItf						m_player_volume;

	uint32_t						m_processed_buffers;
	
	uint16_t* 						m_data;
	size_t	  						m_size;
	uint32_t  						m_sample_rate;
	uint32_t  						m_channels;
	uint32_t  						m_bits;

	bool							m_playing  :1;
	bool							m_looping :1;
	bool							m_streaming :1;

	OggDecoder						m_decoder;
};

//-----------------------------------------------------------------------------
inline void Sound::buffer_callback(SLAndroidSimpleBufferQueueItf caller, void* sound)
{
	Sound* s = (Sound*)sound;

	s->m_processed_buffers++;

	if (s->m_streaming)
	{
		if (s->m_decoder.stream())
		{
			(*s->m_player_bufferqueue)->Enqueue(s->m_player_bufferqueue, s->m_decoder.data(), s->m_decoder.size());
		}
		else if (s->m_looping)
		{
			s->m_decoder.rewind();
			s->m_decoder.stream();
			(*s->m_player_bufferqueue)->Enqueue(s->m_player_bufferqueue, s->m_decoder.data(), s->m_decoder.size());	
		}
	}
}

//-----------------------------------------------------------------------------
inline Sound::Sound() : 
	m_res(NULL),

	m_processed_buffers(0),
	m_data(NULL),
	m_size(0),
	m_sample_rate(0),
	m_channels(0),
	m_bits(0),

	m_playing(false),
	m_looping(false),
	m_streaming(false)
{
}

//-----------------------------------------------------------------------------
inline void Sound::create(SLEngineItf engine, SLObjectItf out_mix_obj, SoundResource* resource)
{
	CE_ASSERT_NOT_NULL(resource);

	m_engine = engine;
	m_out_mix_obj = out_mix_obj;

	m_res = resource;

	SLresult result;

	int32_t speakers = SL_SPEAKER_FRONT_CENTER;

	// Configures buffer queue
	SLDataLocator_AndroidSimpleBufferQueue buffer_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

	// Configures audio format
	SLDataFormat_PCM format_pcm;
	format_pcm.formatType = SL_DATAFORMAT_PCM;
	format_pcm.numChannels = 1;	// Mono
	format_pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
	format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
	format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
	format_pcm.channelMask = speakers;
	format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

	// Configures audio source
	SLDataSource audio_source;
	audio_source.pLocator = &buffer_queue;
	audio_source.pFormat = &format_pcm;

	// Configures audio output mix
	SLDataLocator_OutputMix out_mix;
	out_mix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
	out_mix.outputMix = m_out_mix_obj;

	// Configures audio sink
	SLDataSink audio_sink;
	audio_sink.pLocator = &out_mix;
	audio_sink.pFormat = NULL;

	// Creates sound player
	const SLInterfaceID ids[] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
	const SLboolean reqs[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

	result = (*m_engine)->CreateAudioPlayer(m_engine, &m_player_obj, &audio_source, &audio_sink, 3, ids, reqs);
	check_sles_errors(result);

	result = (*m_player_obj)->Realize(m_player_obj, SL_BOOLEAN_FALSE);
	check_sles_errors(result);

	// Gets interfaces
	result = (*m_player_obj)->GetInterface(m_player_obj, SL_IID_PLAY, &m_player_play);
	check_sles_errors(result);

	result = (*m_player_obj)->GetInterface(m_player_obj, SL_IID_BUFFERQUEUE, &m_player_bufferqueue);
	check_sles_errors(result);

	result = (*m_player_obj)->GetInterface(m_player_obj, SL_IID_VOLUME, &m_player_volume);
	check_sles_errors(result);

	(*m_player_bufferqueue)->RegisterCallback(m_player_bufferqueue, Sound::buffer_callback, this);

	// Manage simple sound or stream
	m_streaming = m_res->sound_type() == ST_OGG;

	if (m_streaming)
	{
		m_decoder.init((char*)m_res->data(), m_res->size());

		m_decoder.stream();
		(*m_player_bufferqueue)->Enqueue(m_player_bufferqueue, m_decoder.data(), m_decoder.size());
	}
	else
	{
		(*m_player_bufferqueue)->Enqueue(m_player_bufferqueue, m_res->data(), m_res->size());
	}
}

//-----------------------------------------------------------------------------
inline void Sound::update()
{
	// not needed right now
}

//-----------------------------------------------------------------------------
inline void Sound::destroy()
{
	if (m_player_obj)
	{
		SLuint32 state;
		(*m_player_obj)->GetState(m_player_obj, &state);

		if (state == SL_OBJECT_STATE_REALIZED)
		{
			(*m_player_bufferqueue)->Clear(m_player_bufferqueue);
			(*m_player_obj)->AbortAsyncOperation(m_player_obj);
			(*m_player_obj)->Destroy(m_player_obj);

			m_player_obj = NULL;
			m_player_play = NULL;
			m_player_bufferqueue = NULL;
			m_player_volume = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
inline void Sound::play()
{
	SLresult result = (*m_player_play)->SetPlayState(m_player_play, SL_PLAYSTATE_PLAYING);
	check_sles_errors(result);

	m_playing = true;
}

//-----------------------------------------------------------------------------
inline void Sound::pause()
{
	SLresult result = (*m_player_play)->SetPlayState(m_player_play, SL_PLAYSTATE_PAUSED);
	check_sles_errors(result);

	m_playing = false;
}

//-----------------------------------------------------------------------------
inline void Sound::loop(bool loop)
{
	m_looping = true;
}

//-----------------------------------------------------------------------------
inline void Sound::set_min_distance(const float min_distance)
{
	Log::w("stub");
}

//-----------------------------------------------------------------------------
inline void Sound::set_max_distance( const float max_distance)
{
	Log::w("stub");
}

//-----------------------------------------------------------------------------
inline void Sound::set_position(const Vec3& pos)
{
	Log::w("stub");
}

//-----------------------------------------------------------------------------
inline void Sound::set_velocity(const Vec3& vel)
{
	Log::w("stub");
}

//-----------------------------------------------------------------------------
inline void Sound::set_direction(const Vec3& dir)
{
	Log::w("stub");
}

//-----------------------------------------------------------------------------
inline void Sound::set_pitch(const float pitch)
{
	Log::w("stub");
}

//-----------------------------------------------------------------------------
inline void Sound::set_gain(const float gain)
{
	Log::w("stub");
}

//-----------------------------------------------------------------------------
inline void Sound::set_rolloff(const float rolloff)
{
	Log::w("stub");
}

//-----------------------------------------------------------------------------
inline float Sound::min_distance() const
{
	Log::w("stub");

	return 0;
}

//-----------------------------------------------------------------------------
inline float Sound::max_distance() const
{
	Log::w("stub");

	return 0;
}

//-----------------------------------------------------------------------------
inline Vec3 Sound::position() const
{
	Log::w("stub");

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
inline Vec3 Sound::velocity() const
{
	Log::w("stub");

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
inline Vec3 Sound::direction() const
{
	Log::w("stub");

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
inline float Sound::pitch() const
{
	Log::w("stub");

	return 0;
}

//-----------------------------------------------------------------------------
inline float Sound::gain() const
{
	Log::w("stub");

	return 0;
}

//-----------------------------------------------------------------------------
inline float Sound::rolloff() const
{
	Log::w("stub");

	return 0;
}

//-----------------------------------------------------------------------------
inline bool Sound::is_playing() const
{
	SLuint32 state;

	(*m_player_play)->GetPlayState(m_player_play, &state);

	return state == SL_PLAYSTATE_PLAYING;
}

//-----------------------------------------------------------------------------
inline int32_t Sound::queued_buffers()
{
	SLAndroidSimpleBufferQueueState state;

	(*m_player_bufferqueue)->GetState(m_player_bufferqueue, &state);

	return state.count;
}

//-----------------------------------------------------------------------------
inline int32_t Sound::processed_buffers()
{
	return m_processed_buffers;
}

}// namespace crown