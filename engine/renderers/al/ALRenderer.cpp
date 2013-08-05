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

#include "ALRenderer.h"
#include "StringUtils.h"

namespace crown
{


//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
#ifdef CROWN_DEBUG
	#define AL_CHECK(function)\
		function;\
		do { ALenum error; CE_ASSERT((error = alGetError()) == AL_NO_ERROR,\
				"OpenAL error: %s", al_error_to_string(error)); } while (0)
#else
	#define AL_CHECK(function)\
		function;
#endif

//-----------------------------------------------------------------------------
ALRenderer::ALRenderer() :
	m_buffers_id_table(m_allocator, MAX_BUFFERS),
	m_sources_id_table(m_allocator, MAX_SOURCES)
{

}

//-----------------------------------------------------------------------------
void ALRenderer::init()
{
	m_device = alcOpenDevice(NULL);
	
	if (!m_device)
	{
		CE_ASSERT(false, "Cannot open audio device");
	}

	m_context = alcCreateContext(m_device, NULL);

	if (!m_context)
	{
		CE_ASSERT(false, "Cannot create context");		
	}

	AL_CHECK(alcMakeContextCurrent(m_context));
}

//-----------------------------------------------------------------------------
void ALRenderer::shutdown()
{
    AL_CHECK(alcDestroyContext(m_context));
    AL_CHECK(alcCloseDevice(m_device));
}

//-----------------------------------------------------------------------------
SoundListener create_listener(float gain, Vec3 position, Vec3 velocity, Vec3 orientation_up, Vec3 orientation_at)
{
	AL_CHECK(alListener3f(AL_POSITION, position.x, position.y, position.z));
	AL_CHECK(alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z));

	ALfloat orientation[] = {	
								orientation_up.x, 
								orientation_up.y, 
								orientation_up.z,
								orientation_at.x, 
								orientation_at.y, 
								orientation_at.z
							};

	AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
}

//-----------------------------------------------------------------------------
SoundBufferId ALRenderer::create_buffer(const void* data, uint32_t size, uint32_t sample_rate, uint32_t channels, uint32_t bxs)
{
	SoundBufferId id = m_buffers_id_table.create();

	SoundBuffer& al_buffer = m_buffers[id.index];

	// Generates AL buffer
	AL_CHECK(alGenBuffers(1, &al_buffer.id));

	bool stereo = (channels > 1);

	// Sets sound's format
	switch(bxs)
	{
		case 8:
		{
			if (stereo)
			{
				al_buffer.format = AL_FORMAT_STEREO8;
			}
			else
			{
				al_buffer.format = AL_FORMAT_MONO8;
			}

			break;
		}

		case 16:
		{
			if (stereo)
			{
				al_buffer.format = AL_FORMAT_STEREO16;
			}
			else
			{
				al_buffer.format = AL_FORMAT_MONO16;
			}

			break;
		}

		default:
		{
			CE_ASSERT(false, "Wrong number of bits per sample.");
			break;
		}
	}

	// Sets sound's size
	al_buffer.size = size;

	// Sets sound's frequency
	al_buffer.freq = sample_rate;

	// Fills AL buffer
	AL_CHECK(alBufferData(al_buffer.id, al_buffer.format, data, al_buffer.size, al_buffer.freq));
}

//-----------------------------------------------------------------------------
void ALRenderer::destroy_buffer(SoundBufferId id)
{
	CE_ASSERT(m_buffers_id_table.has(id), "SoundBuffer does not exist");

	SoundBuffer& al_buffer = m_buffers[id.index];

	AL_CHECK(alDeleteBuffers(1, &al_buffer.id));
}

//-----------------------------------------------------------------------------
SoundSourceId ALRenderer::create_source(Vec3 position, Vec3 velocity, Vec3 direction)
{
	SoundSourceId id = m_sources_id_table.create();

	SoundSource& al_source = m_sources[id.index];

	// Creates AL source
	AL_CHECK(alGenSources(1, &al_source.id));

    AL_CHECK(alSourcef(al_source.id, AL_GAIN, 1));

    AL_CHECK(alSourcef(al_source.id, AL_MIN_GAIN, 0.0f));

    AL_CHECK(alSourcef(al_source.id, AL_MAX_GAIN, 1.0f));

	AL_CHECK(alSource3f(al_source.id, AL_POSITION, position.x, position.y, position.z));

	AL_CHECK(alSource3f(al_source.id, AL_VELOCITY, velocity.x, velocity.y, velocity.z));

	AL_CHECK(alSource3f(al_source.id, AL_DIRECTION, direction.x, direction.y, direction.z));

	AL_CHECK(alSourcei(al_source.id, AL_LOOPING, AL_FALSE));

	return id;
}

//-----------------------------------------------------------------------------
void ALRenderer::play_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	AL_CHECK(alSourcePlay(al_source.id));
}

//-----------------------------------------------------------------------------
void ALRenderer::pause_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	if (is_source_playing(id))
	{
		AL_CHECK(alSourcePause(al_source.id));
	}
}

//-----------------------------------------------------------------------------
void ALRenderer::destroy_source(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	alDeleteSources(1, &al_source.id);
}


//-----------------------------------------------------------------------------
bool ALRenderer::is_source_playing(SoundSourceId id)
{
	CE_ASSERT(m_sources_id_table.has(id), "SoundSource does not exist");

	SoundSource& al_source = m_sources[id.index];

	ALint source_state;
	alGetSourcei(al_source.id, AL_SOURCE_STATE, &source_state);

	return source_state == AL_PLAYING;
}

} // namespace crown