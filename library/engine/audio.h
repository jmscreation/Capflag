#ifndef __ENGINE_AUDIO_H__
#define __ENGINE_AUDIO_H__

namespace Engine {

	class AudioContext;
	class SoundBuffer;
	class SoundInstance;

	class AudioContext {
		static AudioContext* currentCtx;

		std::vector<SoundInstance*> playlist;
		sf::Mutex locked;
		int sampleRate;
		PaStream* stream;
	public:
		AudioContext(int sampleRate=48000);
		virtual ~AudioContext();

		static AudioContext& current() { return *currentCtx; }

	private:
		static int callbackStatic(const void* in,void* out,unsigned long fpb,
				const PaStreamCallbackTimeInfo* tmi,PaStreamCallbackFlags flgs,void* dat);
		int callback(const void* in,void* out,unsigned long fpb,
				const PaStreamCallbackTimeInfo* tmi,PaStreamCallbackFlags flgs);

		friend class SoundBuffer;
		friend class SoundInstance;
	};

	class SoundBuffer {
		short* samples;
		int sampleCount;
		int sampleRate, channels;
		float sampleFactor;
		float defVolume;
		bool defDestroy;

	public:
		SoundBuffer();
		virtual ~SoundBuffer();

		bool loadOGGFromFile(const std::string& fn);
		bool loadOGGFromMemory(const void* data,size_t size);
		bool loadRawFromMemory(const void* data,size_t size,int sampleRate=0,int channels=2,int bytes=2);

		int sampleLength();
		float length();
		void defaultDestroy(bool defdest) { defDestroy=defdest; }
		bool defaultDestroy() { return defDestroy; }
		void defaultVolume(float defvol) { defVolume=defvol; }
		float defaultVolume() { return defVolume; }

		SoundInstance* play();
		SoundInstance* play(bool dest);
		SoundInstance* loop();
		SoundInstance* loop(bool dest);
		SoundInstance* create();
		SoundInstance* create(bool dest);

	private:
		void sample(float& L,float& R,const double& p);

		friend class SoundInstance;
	};

	class SoundInstance {
		SoundBuffer* sound;
		double pos;
		float speed, volPan, vol;
		bool destroy,looping;
	public:
		SoundInstance(SoundBuffer* buf,int playmode=0,bool destroy=false,float volume=1);
		virtual ~SoundInstance();

		void play(float spd=1);
		void loop(float spd=1);
		void pause();
		void stop(bool dest=false);

		bool isPlaying() { return (speed != 0); }

		void pan(float pan) { volPan=pan<-1 ? -1 : pan>1 ? 1 : pan; }
		float pan() { return volPan; }
		void volume(float v) { vol=v<0 ? 0 : v; }
		float volume() { return vol; }

		int sampleLength() { return sound->sampleLength(); }
		int samplePosition();
		void samplePosition(int pos);
		float length() { return sound->length(); }
		double position();
		void position(double pos);

	private:
		void sample(float& L,float& R);

		friend class SoundBuffer;
		friend class AudioContext;
	};

}

#endif // __ENGINE_AUDIO_H__
