/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#ifndef ARX_AUDIO_AMBIANCE_H
#define ARX_AUDIO_AMBIANCE_H

#include "audio/AudioTypes.h"
#include "io/FilePath.h"

namespace audio {

class Ambiance {
	
public:
	
	Ambiance(const fs::path & name);
	~Ambiance();
	
	aalError load();
	
	inline void setUserData(void * _data) { data = _data; }
	inline void * getUserData() const { return data; }
	
	const Channel & getChannel() const { return channel; }
	const fs::path & getName() const { return name; }
	
	aalError setVolume(float volume);
	
	inline bool isPaused() const { return status == Paused; }
	inline bool isPlaying() const { return status == Playing; }
	inline bool isIdle() const { return status == Idle; }
	inline bool isLooped() const { return loop; }
	
	aalError play(const Channel & channel, bool loop = true, size_t fade_interval = 0);
	aalError stop(size_t fade_interval = 0);
	aalError pause();
	aalError resume();
	aalError update();
	
	aalError muteTrack(const std::string & track, bool mute);
	
	void setId(AmbianceId id);
	
	struct Track;
	
private:
	
	static void OnAmbianceSampleEnd(void *, const SourceId &, void * data);
	
	enum Fade {
		None,
		FadeUp,
		FadeDown
	};
	
	enum Status {
		Idle,
		Playing,
		Paused
	};
	
	Status status;
	bool loop;
	Fade fade;
	
	Channel channel;
	float fade_time, fade_interval, fade_max;
	s32 start, time;
	size_t track_c;
	Track * track_l;
	fs::path name;
	
	void * data;
	
};

} // namespace audio

#endif // ARX_AUDIO_AMBIANCE_H
