#include "Game.h"
#include "TextureManager.h"
#include "GameObject.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

/*SDL_Texture* playerTex;
SDL_Rect srcR, destR;*/

GameObject* player;
struct EnemyState
{
	GameObject* obj = nullptr;
	bool active = false;
	int respawnFrames = 0;
	int flickerFrames = 0;
	bool nearMissAwarded = false;
	int prevX = 0;
	int direction = -1;
};
std::vector<EnemyState> enemies;
struct ButterflyState
{
	bool active = false;
	float x = 0.0f;
	float y = 0.0f;
	float phase = 0.0f;
	int frame = 0;
	int frameTick = 0;
	int respawnFrames = 0;
	int direction = -1;
	float speedScale = 1.0f;
};
std::vector<ButterflyState> butterflies;
int playerFlickerFrames = 0;
SDL_Rect prevPlayerRect = { 0, 0, 0, 0 };
int scoreCounter = 0;
Uint32 gameStartTicks = 0;
float cloudParallaxX = 0.0f;
float treeParallaxX = 0.0f;
float skyParallaxX = 0.0f;
float flowerParallaxX = 0.0f;

SDL_Texture* butterflyTex = nullptr;
SDL_Texture* splashTex = nullptr;
int butterflyCapturedCount = 0;
int butterflyFxFrames = 0;
int butterflyFxX = 0;
int butterflyFxY = 0;

int playerInvulnFrames = 0;
int comboChain = 0;
int freezeFrames = 0;
int shakeFrames = 0;
int floatingScoreFrames = 0;
int floatingScoreX = 0;
int floatingScoreY = 0;
int floatingScoreValue = 0;
bool isPaused = false;
float butterflySpeed = 3.0f;
int enemySpeed = 2;
Uint32 lastUpdateTicks = 0;
float gameClockSeconds = 0.0f;
int elapsedSeconds = 0;
int currentTimeLeft = 0;
int hitDangerFrames = 0;
int butterflyGoal = 10;
int levelNumber = 1;
bool levelCleared = false;
int levelClearFrames = 0;
int levelRestartFrames = 0;
bool gameCompleted = false;
bool highScoreLoaded = false;
bool nameEntryActive = false;
int highScoreValue = 0;
std::string highScoreName = "ANON";
std::string nameEntryBuffer;
int stompImpactFrames = 0;
int butterflyWinFrames = 0;
bool gameOver = false;

static const int AUDIO_SAMPLE_RATE = 44100;
static const int AUDIO_VOICES = 16;
static SDL_AudioDeviceID audioDevice = 0;

struct SynthVoice
{
	bool active = false;
	int remaining = 0;
	int total = 0;
	int waveform = 0;
	double frequency = 0.0;
	double phase = 0.0;
	double volume = 0.0;
	double decay = 0.0;
};

struct MusicNote
{
	double frequency;
	int duration;
	int waveform;
	double volume;
};

static std::array<SynthVoice, AUDIO_VOICES> audioVoices;
static const std::array<MusicNote, 16> MUSIC_PATTERN = {
	MusicNote{ 392.0, 240, 0, 0.028 },
	MusicNote{ 440.0, 240, 0, 0.028 },
	MusicNote{ 523.25, 240, 1, 0.026 },
	MusicNote{ 587.33, 240, 0, 0.028 },
	MusicNote{ 659.25, 240, 0, 0.028 },
	MusicNote{ 587.33, 240, 1, 0.026 },
	MusicNote{ 523.25, 240, 0, 0.028 },
	MusicNote{ 494.0, 240, 1, 0.026 },
	MusicNote{ 440.0, 240, 0, 0.028 },
	MusicNote{ 392.0, 240, 0, 0.028 },
	MusicNote{ 523.25, 240, 1, 0.026 },
	MusicNote{ 587.33, 240, 0, 0.028 },
	MusicNote{ 659.25, 240, 0, 0.028 },
	MusicNote{ 783.99, 240, 1, 0.026 },
	MusicNote{ 659.25, 240, 0, 0.028 },
	MusicNote{ 587.33, 360, 0, 0.028 }
};
static const std::array<MusicNote, 8> CLEAR_MUSIC_PATTERN = {
	MusicNote{ 523.25, 180, 0, 0.05 },
	MusicNote{ 659.25, 180, 0, 0.05 },
	MusicNote{ 783.99, 180, 0, 0.055 },
	MusicNote{ 1046.5, 240, 0, 0.06 },
	MusicNote{ 783.99, 180, 0, 0.055 },
	MusicNote{ 1046.5, 180, 0, 0.06 },
	MusicNote{ 1318.5, 220, 0, 0.065 },
	MusicNote{ 1568.0, 320, 0, 0.07 }
};
static int musicPatternIndex = 0;
static int musicSamplesLeft = 0;
static double musicPhase = 0.0;
static double musicFrequency = 0.0;
static int musicWaveform = 0;
static double musicVolume = 0.0;
static int musicTrackMode = 0;

static bool levelTransitionActive = false;

static void StartMusicNote();
static void PlayTone(double frequency, int durationMs, double volume, int waveform, double decay);
static void PlayStompSfx();
static void PlayButterflyCaptureSfx();
static void PlayEnemyHitSfx();
static void PlayWinSfx();
static void PlayGameOverSfx();
static void ResetLevelState(bool incrementLevel);
static void AudioCallback(void* userdata, Uint8* stream, int len);

static const int SCREEN_WIDTH = 1024;
static const int SCREEN_HEIGHT = 768;
static const int GROUND_HEIGHT = 120;
static const int SPRITE_SIZE = 150;
static const int LEVEL_TIME_SECONDS = 300;
static const int BUTTERFLY_SIZE = 150;
static const int BUTTERFLY_FRAMES = 6;
static const int TARGET_BUTTERFLIES_BASE = 10;
static const int MAX_LEVELS = 4;
static const int MAX_NAME_LENGTH = 12;
static const char* HIGH_SCORE_FILE_NAME = "PrincessOwliviaCB-highscore.xml";

static std::string EscapeXml(const std::string& value);
static std::string UnescapeXml(const std::string& value);
static std::string ExtractXmlTag(const std::string& xml, const char* tagName);
static std::string SanitizePlayerName(const std::string& raw);
static void LoadHighScore();
static void SaveHighScore(const std::string& playerName, int score);
static void BeginHighScoreEntry();
static void FinishHighScoreEntry();
static int ActiveEnemySlotsForLevel();
static int ActiveButterflySlotsForLevel();
static int CalcEnemySpeed(float progress);
static int CalcEnemyRespawnFrames(float progress);
static void SpawnEnemy(EnemyState& enemyState, float progress, bool immediate);
static int CalcButterflyRespawnFrames(int elapsed, float progress);
static float CalcButterflySpeed(float progress);
static void SpawnButterfly(ButterflyState& butterflyState, int minuteStage, float progress, bool immediate, int staggerFrames);

static float TriangleWave(float t)
{
	float f = std::fmod(t, 1.0f);
	if (f < 0.0f)
	{
		f += 1.0f;
	}

	if (f < 0.5f)
	{
		return f * 4.0f - 1.0f;
	}

	return 3.0f - f * 4.0f;
}

static int CalcButterflyRespawnFrames(int elapsed, float progress)
{
	int stage30 = elapsed / 30;
	int levelBoost = (levelNumber - 1) * 8;
	int paceBoost = (int)std::floor(progress * 26.0f);
	return std::max(24, 140 - stage30 * 8 - levelBoost - paceBoost);
}

static int ActiveEnemySlotsForLevel()
{
	return std::clamp(levelNumber, 1, 4);
}

static int ActiveButterflySlotsForLevel()
{
	if (levelNumber <= 1) return 1;
	if (levelNumber == 2) return 2;
	return 3;
}

static int CalcEnemySpeed(float progress)
{
	int baseSpeed = 2 + (levelNumber - 1);
	int scaledSpeed = baseSpeed + (int)std::floor(progress * (5.0f + (float)(levelNumber - 1)));
	return std::clamp(scaledSpeed, 2, 14);
}

static int CalcEnemyRespawnFrames(float progress)
{
	int levelBoost = (levelNumber - 1) * 6;
	int minFrames = std::max(6, 20 - levelBoost);
	int maxFrames = std::max(minFrames + 8, 56 - levelBoost * 2);
	int variableFrames = (int)std::floor(progress * 26.0f);
	return std::max(minFrames, maxFrames - variableFrames);
}

static void SpawnEnemy(EnemyState& enemyState, float progress, bool immediate)
{
	if (!enemyState.obj)
	{
		return;
	}

	const int spawnOffset = std::max(36, 210 - (int)std::floor(progress * 160.0f));
	enemyState.direction = (std::rand() % 2 == 0) ? -1 : 1;
	const int spawnX = (enemyState.direction < 0) ? (SCREEN_WIDTH + spawnOffset) : (-SPRITE_SIZE - spawnOffset);
	enemyState.obj->SetPosition(spawnX, SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE);
	enemyState.active = true;
	enemyState.flickerFrames = 0;
	enemyState.nearMissAwarded = false;
	enemyState.prevX = spawnX;
	enemyState.respawnFrames = immediate ? 0 : CalcEnemyRespawnFrames(progress);
}

static float CalcButterflySpeed(float progress)
{
	float baseSpeed = 3.0f + 0.5f * (float)(levelNumber - 1);
	float scaled = baseSpeed + (2.6f + 0.45f * (float)(levelNumber - 1)) * progress;
	return std::min(9.0f, scaled);
}

static void SpawnButterfly(ButterflyState& butterflyState, int minuteStage, float progress, bool immediate, int staggerFrames)
{
	butterflyState.direction = (std::rand() % 2 == 0) ? -1 : 1;
	int spawnOffset = std::max(60, 220 - (int)std::floor(progress * 150.0f));
	butterflyState.x = (butterflyState.direction < 0)
		? (float)(SCREEN_WIDTH + spawnOffset)
		: (float)(-BUTTERFLY_SIZE - spawnOffset);
	float centerY = (minuteStage % 2 == 0) ? 160.0f : 210.0f;
	float amplitudeY = (minuteStage % 2 == 0) ? 72.0f : 92.0f;
	butterflyState.phase += 0.23f + (float)((std::rand() % 7) + 1) * 0.09f;
	butterflyState.y = centerY + TriangleWave(butterflyState.phase) * amplitudeY;
	butterflyState.frame = std::rand() % BUTTERFLY_FRAMES;
	butterflyState.frameTick = 0;
	butterflyState.speedScale = 0.88f + (float)(std::rand() % 35) / 100.0f;
	butterflyState.active = true;
	butterflyState.respawnFrames = immediate ? 0 : (CalcButterflyRespawnFrames(elapsedSeconds, progress) + staggerFrames);
}

static void DrawButterflyIcon(SDL_Renderer* renderer, int x, int y, int p)
{
	SDL_Rect r;
	SDL_SetRenderDrawColor(renderer, 255, 196, 64, 255);
	r = { x + (0 * p), y + (1 * p), 2 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (4 * p), y + (1 * p), 2 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (1 * p), y + (0 * p), 1 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (4 * p), y + (0 * p), 1 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, 250, 120, 64, 255);
	r = { x + (2 * p), y + (1 * p), 2 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (2 * p), y + (3 * p), 1 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
}

static void DrawButterflyFx(SDL_Renderer* renderer, int cx, int cy, int frame)
{
	const SDL_Color colors[4] = {
		{ 255, 252, 112, 255 },
		{ 255, 157, 60, 255 },
		{ 255, 255, 255, 255 },
		{ 255, 84, 84, 255 }
	};
	const SDL_Color c = colors[(frame / 3) % 4];
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

	SDL_Rect r;
	r = { cx - 2, cy - 38, 4, 14 }; SDL_RenderFillRect(renderer, &r);
	r = { cx - 2, cy + 24, 4, 14 }; SDL_RenderFillRect(renderer, &r);
	r = { cx - 38, cy - 2, 14, 4 }; SDL_RenderFillRect(renderer, &r);
	r = { cx + 24, cy - 2, 14, 4 }; SDL_RenderFillRect(renderer, &r);
	r = { cx - 24, cy - 24, 10, 10 }; SDL_RenderFillRect(renderer, &r);
	r = { cx + 14, cy - 24, 10, 10 }; SDL_RenderFillRect(renderer, &r);
	r = { cx - 24, cy + 14, 10, 10 }; SDL_RenderFillRect(renderer, &r);
	r = { cx + 14, cy + 14, 10, 10 }; SDL_RenderFillRect(renderer, &r);
}

static void StartMusicNote()
{
	if (musicTrackMode == 2)
	{
		musicSamplesLeft = 0;
		return;
	}

	const MusicNote* track = nullptr;
	int trackSize = 0;
	bool looping = true;
	if (musicTrackMode == 1)
	{
		track = CLEAR_MUSIC_PATTERN.data();
		trackSize = (int)CLEAR_MUSIC_PATTERN.size();
		looping = false;
	}
	else
	{
		track = MUSIC_PATTERN.data();
		trackSize = (int)MUSIC_PATTERN.size();
	}

	if (musicPatternIndex < 0 || musicPatternIndex >= trackSize)
	{
		if (!looping)
		{
			musicTrackMode = 2;
			musicSamplesLeft = 0;
			return;
		}
		musicPatternIndex = 0;
	}

	const MusicNote& note = track[(size_t)musicPatternIndex];
	float pressure = 1.0f - ((float)std::max(0, currentTimeLeft) / (float)LEVEL_TIME_SECONDS);
	float tempoScale = (musicTrackMode == 1) ? 1.0f : (0.68f + pressure * 0.22f);
	musicFrequency = note.frequency;
	musicWaveform = note.waveform;
	musicVolume = note.volume;
	musicSamplesLeft = std::max(1, (int)((note.duration * AUDIO_SAMPLE_RATE / 1000.0) / tempoScale));
	musicPhase = 0.0;
	musicPatternIndex++;
	if (musicPatternIndex >= trackSize && looping)
	{
		musicPatternIndex = 0;
	}
}

static void PlayTone(double frequency, int durationMs, double volume, int waveform, double decay)
{
	if (audioDevice == 0)
	{
		return;
	}

	SDL_LockAudioDevice(audioDevice);
	for (auto& voice : audioVoices)
	{
		if (!voice.active)
		{
			voice.active = true;
			voice.remaining = std::max(1, durationMs * AUDIO_SAMPLE_RATE / 1000);
			voice.total = voice.remaining;
			voice.waveform = waveform;
			voice.frequency = frequency;
			voice.phase = 0.0;
			voice.volume = volume;
			voice.decay = decay;
			break;
		}
	}
	SDL_UnlockAudioDevice(audioDevice);
}

static void PlayStompSfx()
{
	PlayTone(73.0, 140, 0.95, 1, 1.0);
	PlayTone(98.0, 110, 0.72, 0, 1.0);
	PlayTone(49.0, 180, 0.58, 2, 1.0);
}

static void PlayButterflyCaptureSfx()
{
	PlayTone(784.0, 45, 0.55, 2, 1.0);
	PlayTone(1046.5, 45, 0.58, 2, 1.0);
	PlayTone(1318.5, 70, 0.68, 0, 1.0);
}

static void PlayEnemyHitSfx()
{
	PlayTone(740.0, 22, 0.80, 3, 1.0);
	PlayTone(620.0, 22, 0.78, 3, 1.0);
	PlayTone(520.0, 22, 0.76, 3, 1.0);
	PlayTone(110.0, 120, 0.65, 1, 1.0);
}

static void PlayWinSfx()
{
	PlayTone(523.25, 70, 0.52, 0, 1.0);
	PlayTone(659.25, 70, 0.56, 0, 1.0);
	PlayTone(783.99, 70, 0.60, 0, 1.0);
	PlayTone(1046.5, 120, 0.68, 0, 1.0);
	PlayTone(1318.5, 160, 0.60, 0, 1.0);
}

static void PlayGameOverSfx()
{
	PlayTone(392.0, 100, 0.46, 1, 1.0);
	PlayTone(329.63, 120, 0.48, 1, 1.0);
	PlayTone(261.63, 180, 0.52, 1, 1.0);
	PlayTone(164.81, 240, 0.58, 2, 1.0);
}

static void ResetLevelState(bool incrementLevel)
{
	if (incrementLevel)
	{
		levelNumber++;
	}
	else
	{
		scoreCounter = 0;
	}

	musicTrackMode = 0;
	musicPatternIndex = 0;
	musicSamplesLeft = 0;
	musicPhase = 0.0;

	butterflyGoal = 10 + (levelNumber - 1) * 5;
	if (butterflyGoal > 25)
	{
		butterflyGoal = 25;
	}

	butterflyCapturedCount = 0;
	comboChain = 0;
	playerFlickerFrames = 0;
	playerInvulnFrames = 0;
	butterflyFxFrames = 0;
	stompImpactFrames = 0;
	butterflyWinFrames = 0;
	floatingScoreFrames = 0;
	floatingScoreValue = 0;
	isPaused = false;
	freezeFrames = 0;
	shakeFrames = 0;
	hitDangerFrames = 0;
	levelCleared = false;
	levelClearFrames = 0;
	levelRestartFrames = 0;
	gameCompleted = false;
	nameEntryActive = false;
	nameEntryBuffer.clear();
	SDL_StopTextInput();
	gameOver = false;
	levelTransitionActive = false;
	currentTimeLeft = LEVEL_TIME_SECONDS;
	gameClockSeconds = 0.0f;
	elapsedSeconds = 0;
	gameStartTicks = SDL_GetTicks();
	lastUpdateTicks = SDL_GetTicks();
	cloudParallaxX = 0.0f;
	treeParallaxX = 0.0f;
	const int activeButterflies = ActiveButterflySlotsForLevel();
	for (size_t i = 0; i < butterflies.size(); ++i)
	{
		ButterflyState& butterflyState = butterflies[i];
		if ((int)i < activeButterflies)
		{
			if (i == 0)
			{
				SpawnButterfly(butterflyState, 0, 0.0f, true, 0);
			}
			else
			{
				butterflyState.active = false;
				butterflyState.respawnFrames = 24 + (int)i * 20;
			}
		}
		else
		{
			butterflyState.active = false;
			butterflyState.respawnFrames = 0;
		}
	}
	musicPatternIndex = 0;
	musicSamplesLeft = 0;
	musicPhase = 0.0;
	const int slotsActive = ActiveEnemySlotsForLevel();
	for (size_t i = 0; i < enemies.size(); ++i)
	{
		EnemyState& enemyState = enemies[i];
		if (!enemyState.obj)
		{
			continue;
		}
		enemyState.obj->SetVelocity(0, 0);
		if ((int)i < slotsActive)
		{
			if (i == 0)
			{
				SpawnEnemy(enemyState, 0.0f, true);
			}
			else
			{
				enemyState.active = false;
				enemyState.flickerFrames = 0;
				enemyState.nearMissAwarded = false;
				enemyState.respawnFrames = 10 + (int)i * 14;
			}
		}
		else
		{
			enemyState.active = false;
			enemyState.flickerFrames = 0;
			enemyState.nearMissAwarded = false;
			enemyState.respawnFrames = 0;
			enemyState.obj->SetPosition(SCREEN_WIDTH + 260 + (int)i * 80, SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE);
		}
	}
	if (player)
	{
		player->SetPosition(0, SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE);
	}
	if (audioDevice != 0)
	{
		SDL_LockAudioDevice(audioDevice);
		SDL_UnlockAudioDevice(audioDevice);
	}
}

static void AudioCallback(void* userdata, Uint8* stream, int len)
{
	(void)userdata;
	std::memset(stream, 0, len);
	float* out = reinterpret_cast<float*>(stream);
	int samples = len / sizeof(float);
	if (isPaused)
	{
		return;
	}

	for (int i = 0; i < samples; ++i)
	{
		if (musicSamplesLeft <= 0)
		{
			StartMusicNote();
		}

		double mixed = 0.0;

		if (musicSamplesLeft > 0)
		{
			double phaseStep = musicFrequency / (double)AUDIO_SAMPLE_RATE;
			musicPhase += phaseStep;
			if (musicPhase >= 1.0)
			{
				musicPhase -= 1.0;
			}

			double musicSample = 0.0;
			switch (musicWaveform)
			{
			case 0: musicSample = (musicPhase < 0.5) ? 1.0 : -1.0; break;
			case 1: musicSample = (musicPhase < 0.5) ? (musicPhase * 4.0 - 1.0) : (3.0 - musicPhase * 4.0); break;
			case 2: musicSample = 2.0 * std::fabs(2.0 * musicPhase - 1.0) - 1.0; break;
			case 3: musicSample = (((int)(musicPhase * 16.0)) & 1) ? 1.0 : -1.0; break;
			default: musicSample = (musicPhase < 0.5) ? 1.0 : -1.0; break;
			}
			mixed += musicSample * musicVolume;
			musicSamplesLeft--;
		}

		for (auto& voice : audioVoices)
		{
			if (!voice.active)
			{
				continue;
			}

			double phaseStep = voice.frequency / (double)AUDIO_SAMPLE_RATE;
			voice.phase += phaseStep;
			if (voice.phase >= 1.0)
			{
				voice.phase -= 1.0;
			}

			double sample = 0.0;
			switch (voice.waveform)
			{
			case 0: sample = (voice.phase < 0.5) ? 1.0 : -1.0; break;
			case 1: sample = (voice.phase < 0.5) ? (voice.phase * 4.0 - 1.0) : (3.0 - voice.phase * 4.0); break;
			case 2: sample = 2.0 * std::fabs(2.0 * voice.phase - 1.0) - 1.0; break;
			case 3: sample = (((int)(voice.phase * 16.0)) & 1) ? 1.0 : -1.0; break;
			default: sample = (voice.phase < 0.5) ? 1.0 : -1.0; break;
			}

			double life = (double)voice.remaining / (double)std::max(1, voice.total);
			double envelope = std::max(0.0, life * voice.volume);
			mixed += sample * envelope;
			voice.remaining--;
			if (voice.remaining <= 0)
			{
				voice.active = false;
			}
		}

		mixed = std::clamp(mixed, -1.0, 1.0);
		out[i] = (float)mixed;
	}
}

static void DrawCloud(SDL_Renderer* renderer, int x, int y, int p)
{
	SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
	SDL_Rect r;
	r = { x + (1 * p), y + (0 * p), 6 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (0 * p), y + (1 * p), 8 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (1 * p), y + (3 * p), 6 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);
}

static void DrawTree(SDL_Renderer* renderer, int x, int baseY, int p)
{
	SDL_Rect r;
	SDL_SetRenderDrawColor(renderer, 94, 62, 31, 255);
	r = { x + (4 * p), baseY - (4 * p), 2 * p, 4 * p }; SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, 42, 156, 71, 255);
	r = { x + (2 * p), baseY - (9 * p), 6 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (1 * p), baseY - (7 * p), 8 * p, 2 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (2 * p), baseY - (5 * p), 6 * p, 1 * p }; SDL_RenderFillRect(renderer, &r);
}

static void DrawSun(SDL_Renderer* renderer, int x, int y, int p)
{
	const int radius = 6 * p;
	const int centerX = x + radius;
	const int centerY = y + radius;
	SDL_SetRenderDrawColor(renderer, 255, 238, 100, 255);
	for (int dy = -radius; dy <= radius; ++dy)
	{
		int span = (int)std::sqrt((double)(radius * radius - dy * dy));
		SDL_RenderDrawLine(renderer, centerX - span, centerY + dy, centerX + span, centerY + dy);
	}

	SDL_SetRenderDrawColor(renderer, 255, 220, 72, 255);
	SDL_RenderDrawLine(renderer, centerX + radius + 2, centerY - 2, centerX + radius + 8, centerY - 2);
	SDL_RenderDrawLine(renderer, centerX - radius - 8, centerY - 2, centerX - radius - 2, centerY - 2);
	SDL_RenderDrawLine(renderer, centerX - 2, centerY + radius + 2, centerX - 2, centerY + radius + 8);
	SDL_RenderDrawLine(renderer, centerX - 2, centerY - radius - 8, centerX - 2, centerY - radius - 2);
}

static void DrawRainbow(SDL_Renderer* renderer, int x, int y, int p)
{
	const SDL_Color colors[6] = {
		{ 255, 92, 92, 255 },
		{ 255, 164, 64, 255 },
		{ 255, 236, 96, 255 },
		{ 96, 220, 128, 255 },
		{ 96, 172, 255, 255 },
		{ 184, 120, 255, 255 }
	};
	const int centerX = x + 60 * p;
	const int centerY = y + 34 * p;
	for (int i = 0; i < 6; ++i)
	{
		SDL_SetRenderDrawColor(renderer, colors[i].r, colors[i].g, colors[i].b, colors[i].a);
		const int outerRadius = 30 * p - i * (4 * p);
		const int innerRadius = std::max(0, outerRadius - (4 * p));
		for (int dy = 0; dy <= outerRadius; ++dy)
		{
			int outerSpan = (int)std::sqrt((double)(outerRadius * outerRadius - dy * dy));
			int innerSpan = 0;
			if (dy <= innerRadius)
			{
				innerSpan = (int)std::sqrt((double)(innerRadius * innerRadius - dy * dy));
			}
			SDL_RenderDrawLine(renderer, centerX - outerSpan, centerY - dy, centerX - innerSpan, centerY - dy);
			SDL_RenderDrawLine(renderer, centerX + innerSpan, centerY - dy, centerX + outerSpan, centerY - dy);
		}
	}
}

static void DrawFlowerCluster(SDL_Renderer* renderer, int x, int groundY, int p)
{
	SDL_Rect r;
	SDL_SetRenderDrawColor(renderer, 52, 180, 84, 255);
	r = { x + (4 * p), groundY - (8 * p), 2 * p, 8 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (12 * p), groundY - (10 * p), 2 * p, 10 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (22 * p), groundY - (7 * p), 2 * p, 7 * p }; SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, 255, 84, 140, 255);
	r = { x, groundY - (12 * p), 6 * p, 6 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (2 * p), groundY - (14 * p), 6 * p, 6 * p }; SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, 255, 208, 92, 255);
	r = { x + (10 * p), groundY - (14 * p), 6 * p, 6 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (12 * p), groundY - (16 * p), 6 * p, 6 * p }; SDL_RenderFillRect(renderer, &r);

	SDL_SetRenderDrawColor(renderer, 180, 120, 255, 255);
	r = { x + (20 * p), groundY - (11 * p), 6 * p, 6 * p }; SDL_RenderFillRect(renderer, &r);
	r = { x + (22 * p), groundY - (13 * p), 6 * p, 6 * p }; SDL_RenderFillRect(renderer, &r);
}

static void DrawGlyph(SDL_Renderer* renderer, int x, int y, int scale, char ch)
{
	static const char* G0[7] = { "11111", "10001", "10001", "10001", "10001", "10001", "11111" };
	static const char* G1[7] = { "00100", "01100", "00100", "00100", "00100", "00100", "01110" };
	static const char* G2[7] = { "11111", "00001", "00001", "11111", "10000", "10000", "11111" };
	static const char* G3[7] = { "11111", "00001", "00001", "01111", "00001", "00001", "11111" };
	static const char* G4[7] = { "10001", "10001", "10001", "11111", "00001", "00001", "00001" };
	static const char* G5[7] = { "11111", "10000", "10000", "11111", "00001", "00001", "11111" };
	static const char* G6[7] = { "11111", "10000", "10000", "11111", "10001", "10001", "11111" };
	static const char* G7[7] = { "11111", "00001", "00010", "00100", "01000", "01000", "01000" };
	static const char* G8[7] = { "11111", "10001", "10001", "11111", "10001", "10001", "11111" };
	static const char* G9[7] = { "11111", "10001", "10001", "11111", "00001", "00001", "11111" };
	static const char* GG[7] = { "01110", "10001", "10000", "10011", "10001", "10001", "01110" };
	static const char* GA[7] = { "01110", "10001", "10001", "11111", "10001", "10001", "10001" };
	static const char* GB[7] = { "11110", "10001", "10001", "11110", "10001", "10001", "11110" };
	static const char* GF[7] = { "11111", "10000", "10000", "11110", "10000", "10000", "10000" };
	static const char* GD[7] = { "11110", "10001", "10001", "10001", "10001", "10001", "11110" };
	static const char* GC[7] = { "01111", "10000", "10000", "10000", "10000", "10000", "01111" };
	static const char* GE[7] = { "11111", "10000", "10000", "11110", "10000", "10000", "11111" };
	static const char* GH[7] = { "10001", "10001", "10001", "11111", "10001", "10001", "10001" };
	static const char* GL[7] = { "10000", "10000", "10000", "10000", "10000", "10000", "11111" };
	static const char* GI[7] = { "11111", "00100", "00100", "00100", "00100", "00100", "11111" };
	static const char* GJ[7] = { "00111", "00010", "00010", "00010", "10010", "10010", "01100" };
	static const char* GK[7] = { "10001", "10010", "10100", "11000", "10100", "10010", "10001" };
	static const char* GN[7] = { "10001", "11001", "10101", "10011", "10001", "10001", "10001" };
	static const char* GM[7] = { "10001", "11011", "10101", "10101", "10001", "10001", "10001" };
	static const char* GO[7] = { "01110", "10001", "10001", "10001", "10001", "10001", "01110" };
	static const char* GP[7] = { "11110", "10001", "10001", "11110", "10000", "10000", "10000" };
	static const char* GQ[7] = { "01110", "10001", "10001", "10001", "10101", "10010", "01101" };
	static const char* GR[7] = { "11110", "10001", "10001", "11110", "10100", "10010", "10001" };
	static const char* GS[7] = { "01111", "10000", "10000", "01110", "00001", "00001", "11110" };
	static const char* GT[7] = { "11111", "00100", "00100", "00100", "00100", "00100", "00100" };
	static const char* GU[7] = { "10001", "10001", "10001", "10001", "10001", "10001", "01110" };
	static const char* GV[7] = { "10001", "10001", "10001", "10001", "01010", "01010", "00100" };
	static const char* GW[7] = { "10001", "10001", "10001", "10101", "10101", "10101", "01010" };
	static const char* GX[7] = { "10001", "10001", "01010", "00100", "01010", "10001", "10001" };
	static const char* GY[7] = { "10001", "01010", "00100", "00100", "00100", "00100", "00100" };
	static const char* GZ[7] = { "11111", "00001", "00010", "00100", "01000", "10000", "11111" };
	static const char* GColon[7] = { "00000", "00100", "00100", "00000", "00100", "00100", "00000" };
	static const char* GUnder[7] = { "00000", "00000", "00000", "00000", "00000", "00000", "11111" };
	static const char* GSpace[7] = { "00000", "00000", "00000", "00000", "00000", "00000", "00000" };
	const char** g = GSpace;

	switch (ch)
	{
	case '0': g = G0; break;
	case '1': g = G1; break;
	case '2': g = G2; break;
	case '3': g = G3; break;
	case '4': g = G4; break;
	case '5': g = G5; break;
	case '6': g = G6; break;
	case '7': g = G7; break;
	case '8': g = G8; break;
	case '9': g = G9; break;
	case 'G': g = GG; break;
	case 'A': g = GA; break;
	case 'B': g = GB; break;
	case 'F': g = GF; break;
	case 'D': g = GD; break;
	case 'C': g = GC; break;
	case 'E': g = GE; break;
	case 'H': g = GH; break;
	case 'L': g = GL; break;
	case 'I': g = GI; break;
	case 'J': g = GJ; break;
	case 'K': g = GK; break;
	case 'N': g = GN; break;
	case 'M': g = GM; break;
	case 'O': g = GO; break;
	case 'P': g = GP; break;
	case 'Q': g = GQ; break;
	case 'R': g = GR; break;
	case 'S': g = GS; break;
	case 'T': g = GT; break;
	case 'U': g = GU; break;
	case 'V': g = GV; break;
	case 'W': g = GW; break;
	case 'X': g = GX; break;
	case 'Y': g = GY; break;
	case 'Z': g = GZ; break;
	case ':': g = GColon; break;
	case '_': g = GUnder; break;
	default: break;
	}

	SDL_Rect px;
	for (int row = 0; row < 7; ++row)
	{
		for (int col = 0; col < 5; ++col)
		{
			if (g[row][col] == '1')
			{
				px = { x + col * scale, y + row * scale, scale, scale };
				SDL_RenderFillRect(renderer, &px);
			}
		}
	}
}

static void DrawText(SDL_Renderer* renderer, int x, int y, int scale, const char* text)
{
	if (!text)
	{
		return;
	}

	for (int i = 0; text[i] != '\0'; ++i)
	{
		DrawGlyph(renderer, x + i * (6 * scale), y, scale, text[i]);
	}
}

static std::string EscapeXml(const std::string& value)
{
	std::string escaped;
	for (char ch : value)
	{
		switch (ch)
		{
		case '&': escaped += "&amp;"; break;
		case '<': escaped += "&lt;"; break;
		case '>': escaped += "&gt;"; break;
		case '"': escaped += "&quot;"; break;
		case '\'': escaped += "&apos;"; break;
		default: escaped.push_back(ch); break;
		}
	}
	return escaped;
}

static std::string UnescapeXml(const std::string& value)
{
	std::string unescaped = value;
	struct Entity { const char* from; const char* to; };
	static const Entity entities[] = {
		{ "&amp;", "&" },
		{ "&lt;", "<" },
		{ "&gt;", ">" },
		{ "&quot;", "\"" },
		{ "&apos;", "'" }
	};
	for (const Entity& entity : entities)
	{
		size_t pos = 0;
		while ((pos = unescaped.find(entity.from, pos)) != std::string::npos)
		{
			unescaped.replace(pos, std::strlen(entity.from), entity.to);
			pos += std::strlen(entity.to);
		}
	}
	return unescaped;
}

static std::string ExtractXmlTag(const std::string& xml, const char* tagName)
{
	const std::string openTag = std::string("<") + tagName + ">";
	const std::string closeTag = std::string("</") + tagName + ">";
	const size_t start = xml.find(openTag);
	if (start == std::string::npos)
	{
		return "";
	}
	const size_t valueStart = start + openTag.size();
	const size_t end = xml.find(closeTag, valueStart);
	if (end == std::string::npos)
	{
		return "";
	}
	return UnescapeXml(xml.substr(valueStart, end - valueStart));
}

static std::string SanitizePlayerName(const std::string& raw)
{
	std::string sanitized;
	for (char ch : raw)
	{
		unsigned char value = (unsigned char)ch;
		if (std::isalnum(value))
		{
			sanitized.push_back((char)std::toupper(value));
		}
		else if (std::isspace(value))
		{
			if (!sanitized.empty() && sanitized.back() != ' ')
			{
				sanitized.push_back(' ');
			}
		}
		if ((int)sanitized.size() >= MAX_NAME_LENGTH)
		{
			break;
		}
	}
	while (!sanitized.empty() && sanitized.front() == ' ')
	{
		sanitized.erase(sanitized.begin());
	}
	while (!sanitized.empty() && sanitized.back() == ' ')
	{
		sanitized.pop_back();
	}
	if (sanitized.empty())
	{
		return "ANON";
	}
	return sanitized;
}

static void LoadHighScore()
{
	if (highScoreLoaded)
	{
		return;
	}
	highScoreLoaded = true;

	std::ifstream input(HIGH_SCORE_FILE_NAME);
	if (!input.is_open())
	{
		return;
	}

	std::ostringstream buffer;
	buffer << input.rdbuf();
	const std::string xml = buffer.str();
	const std::string savedName = ExtractXmlTag(xml, "name");
	const std::string savedScore = ExtractXmlTag(xml, "score");
	if (!savedName.empty())
	{
		highScoreName = SanitizePlayerName(savedName);
	}
	if (!savedScore.empty())
	{
		highScoreValue = std::max(0, std::atoi(savedScore.c_str()));
	}
}

static void SaveHighScore(const std::string& playerName, int score)
{
	highScoreName = SanitizePlayerName(playerName);
	highScoreValue = std::max(0, score);

	std::ofstream output(HIGH_SCORE_FILE_NAME, std::ios::trunc);
	if (!output.is_open())
	{
		std::fprintf(stderr, "Failed to write high score file '%s'\n", HIGH_SCORE_FILE_NAME);
		return;
	}

	output << "<highscore>\n";
	output << "  <name>" << EscapeXml(highScoreName) << "</name>\n";
	output << "  <score>" << highScoreValue << "</score>\n";
	output << "</highscore>\n";
}

static void BeginHighScoreEntry()
{
	if (nameEntryActive || scoreCounter <= highScoreValue)
	{
		return;
	}
	nameEntryActive = true;
	nameEntryBuffer.clear();
	SDL_StartTextInput();
}

static void FinishHighScoreEntry()
{
	SaveHighScore(nameEntryBuffer, scoreCounter);
	nameEntryActive = false;
	nameEntryBuffer.clear();
	SDL_StopTextInput();
}

Game::Game()
{}

Game::~Game()
{}

void Game::DismissSplash()
{
	splashActive = false;
	splashFrames = 0;
}

void Game::ResetInputEdges()
{
	input.jumpPressed = false;
	input.pausePressed = false;
	input.restartPressed = false;
}

void Game::UpdateDirectionalInput()
{
	const Uint8* keyState = SDL_GetKeyboardState(NULL);
	input.left = keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_A] || virtualLeftHeld;
	input.right = keyState[SDL_SCANCODE_RIGHT] || keyState[SDL_SCANCODE_D] || virtualRightHeld;
	input.up = keyState[SDL_SCANCODE_UP] || keyState[SDL_SCANCODE_W];
	input.down = keyState[SDL_SCANCODE_DOWN] || keyState[SDL_SCANCODE_S];
}

void Game::init(const char* title, int width, int height, bool fullscreen)
{
	int flags = 0;
	window = nullptr;
	renderer = nullptr;
	player = nullptr;
	enemies.clear();
	butterflies.clear();
	std::srand((unsigned int)SDL_GetTicks());

	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	const Uint32 sdlInitFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS;
	if (SDL_Init(sdlInitFlags) == 0)
	{
		int imgFlags = IMG_Init(IMG_INIT_PNG);
		if ((imgFlags & IMG_INIT_PNG) == 0)
		{
			std::cerr << "IMG_Init PNG failed: " << IMG_GetError() << std::endl;
		}

		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
		if (!window)
		{
			std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
		}

		if (window)
		{
			SDL_Surface* iconSurface = TextureManager::LoadSurface("assets/thumbnail.png");
			if (iconSurface)
			{
				SDL_SetWindowIcon(window, iconSurface);
				SDL_FreeSurface(iconSurface);
			}
			else
			{
				std::cerr << "Failed to load window icon thumbnail" << std::endl;
			}

			Uint32 rendererFlags = SDL_RENDERER_ACCELERATED;
#ifdef BIRCHENGINE_WEB
			rendererFlags = SDL_RENDERER_ACCELERATED;
#else
			rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
#endif
			renderer = SDL_CreateRenderer(window, -1, rendererFlags);
			if (!renderer)
			{
				renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			}
			if (!renderer)
			{
				renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
			}
			if (!renderer)
			{
				renderer = SDL_CreateRenderer(window, -1, 0);
			}
		}

		if (renderer)
		{
			SDL_SetRenderDrawColor(renderer, 107, 185, 240, 255);
			isRunning = true;
		}
		else
		{
			std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
		}
	}
	else
	{
		std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
	}

	if (isRunning)
	{
		LoadHighScore();
		splashTex = TextureManager::LoadTexture("assets/coverart.png", renderer);
		butterflyTex = TextureManager::LoadTexture("assets/coins.png", renderer);
		const int groundY = SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE;
		player = new GameObject("assets/cha11.png", renderer, 0, groundY, 6);
		player->SetGroundY(groundY);
		enemies.reserve(4);
		for (int i = 0; i < 4; ++i)
		{
			EnemyState enemyState;
			enemyState.obj = new GameObject("assets/arche.png", renderer, SCREEN_WIDTH + 240 + i * 80, groundY, 6);
			enemyState.obj->SetGroundY(groundY);
			enemyState.active = false;
			enemies.push_back(enemyState);
		}
		butterflies.reserve(3);
		for (int i = 0; i < 3; ++i)
		{
			ButterflyState butterflyState;
			butterflyState.active = false;
			butterflyState.respawnFrames = 0;
			butterflies.push_back(butterflyState);
		}
		scoreCounter = 0;
		isPaused = false;
		levelNumber = 1;
		butterflyGoal = TARGET_BUTTERFLIES_BASE;
		gameCompleted = false;
		nameEntryActive = false;
		nameEntryBuffer.clear();
		if (audioDevice == 0)
		{
			SDL_AudioSpec want{};
			SDL_AudioSpec have{};
			want.freq = AUDIO_SAMPLE_RATE;
			want.format = AUDIO_F32SYS;
			want.channels = 1;
			want.samples = 2048;
			want.callback = AudioCallback;
			audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
			if (audioDevice != 0)
			{
				SDL_PauseAudioDevice(audioDevice, 0);
			}
		}
		ResetLevelState(false);
		splashActive = true;
		splashFrames = 180;
	}
}

void Game::handleEvents()
{
	SDL_Event event;
	ResetInputEdges();

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_KEYDOWN:
			if (splashActive)
			{
				DismissSplash();
				break;
			}
			if (nameEntryActive)
			{
				if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_RETURN)
				{
					FinishHighScoreEntry();
					if (gameOver || gameCompleted)
					{
						input.restartPressed = true;
					}
				}
				else if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_BACKSPACE && !nameEntryBuffer.empty())
				{
					nameEntryBuffer.pop_back();
				}
				else if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_ESCAPE)
				{
					FinishHighScoreEntry();
				}
				break;
			}
			if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_RETURN)
			{
				if (gameOver)
				{
					input.restartPressed = true;
				}
				else if (!levelCleared && !gameCompleted)
				{
					input.pausePressed = true;
				}
			}
			if (event.key.repeat == 0 && event.key.keysym.sym == SDLK_SPACE)
			{
				input.jumpPressed = true;
			}
			break;
		case SDL_TEXTINPUT:
			if (nameEntryActive)
			{
				for (int i = 0; event.text.text[i] != '\0' && (int)nameEntryBuffer.size() < MAX_NAME_LENGTH; ++i)
				{
					char ch = event.text.text[i];
					unsigned char value = (unsigned char)ch;
					if (std::isalnum(value))
					{
						nameEntryBuffer.push_back((char)std::toupper(value));
					}
					else if (std::isspace(value) && !nameEntryBuffer.empty() && nameEntryBuffer.back() != ' ')
					{
						nameEntryBuffer.push_back(' ');
					}
				}
			}
			break;
		case SDL_FINGERDOWN:
		{
			if (splashActive)
			{
				DismissSplash();
				break;
			}

			float x = event.tfinger.x;
			float y = event.tfinger.y;
			if (y > 0.72f)
			{
				if (x < 0.35f)
				{
					virtualLeftHeld = true;
				}
				else if (x > 0.65f)
				{
					virtualRightHeld = true;
				}
				else
				{
					virtualJumpQueued = true;
				}
			}
			else
			{
				virtualJumpQueued = true;
			}
			break;
		}
		case SDL_FINGERUP:
			virtualLeftHeld = false;
			virtualRightHeld = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				if (splashActive)
				{
					DismissSplash();
					break;
				}

				float x = (float)event.button.x / (float)SCREEN_WIDTH;
				float y = (float)event.button.y / (float)SCREEN_HEIGHT;
				if (y > 0.72f)
				{
					if (x < 0.35f)
					{
						virtualLeftHeld = true;
					}
					else if (x > 0.65f)
					{
						virtualRightHeld = true;
					}
					else
					{
						virtualJumpQueued = true;
					}
				}
				else
				{
					virtualJumpQueued = true;
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				virtualLeftHeld = false;
				virtualRightHeld = false;
			}
			break;
		default:
			break;
		}
	}

	UpdateDirectionalInput();
	if (virtualJumpQueued)
	{
		input.jumpPressed = true;
		virtualJumpQueued = false;
	}
}

void Game::update()
{
	int playerVelX = 0;
	int playerVelY = 0;
	Uint32 nowTicks = SDL_GetTicks();
	if (lastUpdateTicks == 0)
	{
		lastUpdateTicks = nowTicks;
	}

	float dt = (float)(nowTicks - lastUpdateTicks) / 1000.0f;
	if (dt > 0.05f)
	{
		dt = 0.05f;
	}
	lastUpdateTicks = nowTicks;

	if (splashActive)
	{
		return;
	}

	if (input.restartPressed && gameOver && !nameEntryActive)
	{
		ResetLevelState(false);
	}

	if (input.restartPressed && gameCompleted && !nameEntryActive)
	{
		ResetLevelState(false);
	}

	if (input.pausePressed && !gameOver && !levelCleared && !gameCompleted)
	{
		isPaused = !isPaused;
	}

	if (input.jumpPressed && !isPaused && !levelCleared && !gameCompleted && !gameOver && player)
	{
		player->Jump();
	}

	if (isPaused)
	{
		return;
	}

	if (levelCleared)
	{
		if (levelClearFrames > 0)
		{
			levelClearFrames--;
			return;
		}

		if (levelRestartFrames == 0)
		{
			levelRestartFrames = 60;
		}

		levelRestartFrames--;
		if (levelRestartFrames <= 0)
		{
			ResetLevelState(true);
		}
		return;
	}

	if (gameCompleted)
	{
		return;
	}

	if (gameOver)
	{
		return;
	}

	if (freezeFrames > 0)
	{
		freezeFrames--;
		return;
	}

	gameClockSeconds += dt;
	elapsedSeconds = (int)gameClockSeconds;
	currentTimeLeft = LEVEL_TIME_SECONDS - elapsedSeconds;
	if (currentTimeLeft < 0)
	{
		currentTimeLeft = 0;
	}
	if (currentTimeLeft == 0)
	{
		gameOver = true;
		isPaused = false;
		musicTrackMode = 2;
		musicSamplesLeft = 0;
		PlayGameOverSfx();
		BeginHighScoreEntry();
		return;
	}

	float progress = 1.0f - ((float)currentTimeLeft / (float)LEVEL_TIME_SECONDS);
	enemySpeed = CalcEnemySpeed(progress);
	butterflySpeed = CalcButterflySpeed(progress);

	if (input.left)
	{
		playerVelX = -1;
	}
	else if (input.right)
	{
		playerVelX = 1;
	}

	if (input.up)
	{
		playerVelY = -1;
	}
	else if (input.down)
	{
		playerVelY = 1;
	}

	cloudParallaxX -= playerVelX * 0.35f;
	treeParallaxX -= playerVelX * 0.75f;
	skyParallaxX -= playerVelX * 0.12f;
	flowerParallaxX -= playerVelX * 1.10f;
	if (cloudParallaxX <= -SCREEN_WIDTH) cloudParallaxX += SCREEN_WIDTH;
	if (cloudParallaxX >= SCREEN_WIDTH) cloudParallaxX -= SCREEN_WIDTH;
	if (treeParallaxX <= -SCREEN_WIDTH) treeParallaxX += SCREEN_WIDTH;
	if (treeParallaxX >= SCREEN_WIDTH) treeParallaxX -= SCREEN_WIDTH;
	if (skyParallaxX <= -SCREEN_WIDTH) skyParallaxX += SCREEN_WIDTH;
	if (skyParallaxX >= SCREEN_WIDTH) skyParallaxX -= SCREEN_WIDTH;
	if (flowerParallaxX <= -SCREEN_WIDTH) flowerParallaxX += SCREEN_WIDTH;
	if (flowerParallaxX >= SCREEN_WIDTH) flowerParallaxX -= SCREEN_WIDTH;

	if (player)
	{
		prevPlayerRect = player->GetHitbox();
		player->SetVelocity(playerVelX, playerVelY);
		player->Update();

		SDL_Rect playerBounds = player->GetBounds();
		int clampedX = std::clamp(playerBounds.x, 0, SCREEN_WIDTH - playerBounds.w);
		int clampedY = std::clamp(playerBounds.y, 0, SCREEN_HEIGHT - playerBounds.h);
		if (clampedX != playerBounds.x || clampedY != playerBounds.y)
		{
			player->SetPosition(clampedX, clampedY);
			player->Update();
		}
	}

	const int activeEnemySlots = ActiveEnemySlotsForLevel();
	for (size_t i = 0; i < enemies.size(); ++i)
	{
		EnemyState& enemyState = enemies[i];
		if (!enemyState.obj)
		{
			continue;
		}

		if ((int)i >= activeEnemySlots)
		{
			enemyState.active = false;
			enemyState.obj->SetVelocity(0, 0);
			continue;
		}

		if (enemyState.active)
		{
			enemyState.prevX = enemyState.obj->GetBounds().x;
			enemyState.obj->SetVelocity(enemyState.direction * enemySpeed, 0);
			enemyState.obj->Update();
			SDL_Rect enemyRect = enemyState.obj->GetBounds();
			bool enemyExited = (enemyState.direction < 0)
				? (enemyRect.x + enemyRect.w < 0)
				: (enemyRect.x > SCREEN_WIDTH);
			if (enemyExited)
			{
				enemyState.active = false;
				enemyState.respawnFrames = CalcEnemyRespawnFrames(progress);
				enemyState.flickerFrames = 0;
				enemyState.nearMissAwarded = false;
			}
		}
		else
		{
			if (enemyState.respawnFrames > 0)
			{
				enemyState.respawnFrames--;
			}
			else
			{
				SpawnEnemy(enemyState, progress, false);
			}
		}
	}

	if (player)
	{
		SDL_Rect playerRect = player->GetHitbox();
		for (EnemyState& enemyState : enemies)
		{
			if (!enemyState.obj || !enemyState.active || enemyState.flickerFrames > 0)
			{
				continue;
			}

			SDL_Rect enemyRect = enemyState.obj->GetHitbox();
			if (SDL_HasIntersection(&playerRect, &enemyRect))
			{
				const bool descending = playerRect.y > prevPlayerRect.y;
				const bool fromAbove = (prevPlayerRect.y + prevPlayerRect.h) <= (enemyRect.y + 8);
				if (descending && fromAbove)
				{
					PlayStompSfx();
					enemyState.flickerFrames = 40;
					stompImpactFrames = 16;
					int stompBonus = (currentTimeLeft <= 30) ? 200 : 100;
					scoreCounter += stompBonus;
					player->StompBounce();
					freezeFrames = 3;
					floatingScoreValue = stompBonus;
					floatingScoreFrames = 40;
					floatingScoreX = enemyRect.x + enemyRect.w / 2;
					floatingScoreY = enemyRect.y - 20;
				}
				else if (playerFlickerFrames == 0 && playerInvulnFrames == 0)
				{
					PlayEnemyHitSfx();
					playerFlickerFrames = 60;
					playerInvulnFrames = 60;
					scoreCounter = std::max(0, scoreCounter - 100);
					butterflyCapturedCount = std::max(0, butterflyCapturedCount - 1);
					comboChain = 0;
					shakeFrames = 14;
					hitDangerFrames = 22;
					break;
				}
			}
		}

		for (EnemyState& enemyState : enemies)
		{
			if (!enemyState.obj || !enemyState.active || enemyState.nearMissAwarded)
			{
				continue;
			}

			SDL_Rect enemyRect = enemyState.obj->GetHitbox();
			bool crossed = false;
			if (enemyState.direction < 0)
			{
				crossed = (enemyState.prevX >= playerRect.x + playerRect.w) && (enemyRect.x + enemyRect.w < playerRect.x);
			}
			else
			{
				crossed = (enemyState.prevX + enemyRect.w <= playerRect.x) && (enemyRect.x > playerRect.x + playerRect.w);
			}
			bool playerAirborne = playerRect.y < (SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE - 4);
			int verticalGap = std::abs((playerRect.y + playerRect.h) - enemyRect.y);
			if (crossed && playerAirborne && verticalGap < 80)
			{
				scoreCounter += 50;
				enemyState.nearMissAwarded = true;
				floatingScoreValue = 50;
				floatingScoreFrames = 30;
				floatingScoreX = playerRect.x + playerRect.w / 2;
				floatingScoreY = playerRect.y - 14;
			}
		}
	}

	if (playerFlickerFrames > 0) playerFlickerFrames--;
	for (EnemyState& enemyState : enemies)
	{
		if (enemyState.flickerFrames > 0)
		{
			enemyState.flickerFrames--;
		}
	}
	if (playerInvulnFrames > 0) playerInvulnFrames--;
	if (hitDangerFrames > 0) hitDangerFrames--;

	int minuteStage = elapsedSeconds / 60;
	const int activeButterflies = ActiveButterflySlotsForLevel();
	for (size_t i = 0; i < butterflies.size(); ++i)
	{
		ButterflyState& butterflyState = butterflies[i];
		if ((int)i >= activeButterflies)
		{
			butterflyState.active = false;
			continue;
		}

		if (butterflyState.active)
		{
			float centerY = (minuteStage % 2 == 0) ? 160.0f : 210.0f;
			float amplitudeY = (minuteStage % 2 == 0) ? 72.0f : 92.0f;
			float phaseSpeed = (minuteStage % 2 == 0) ? 0.028f : 0.038f;
			butterflyState.x += (float)butterflyState.direction * butterflySpeed * butterflyState.speedScale;
			butterflyState.phase += phaseSpeed;
			butterflyState.y = centerY + TriangleWave(butterflyState.phase) * amplitudeY;
			butterflyState.frameTick++;
			if (butterflyState.frameTick >= 5)
			{
				butterflyState.frameTick = 0;
				butterflyState.frame = (butterflyState.frame + 1) % BUTTERFLY_FRAMES;
			}

			bool butterflyExited = (butterflyState.direction < 0)
				? (butterflyState.x < -BUTTERFLY_SIZE)
				: (butterflyState.x > SCREEN_WIDTH + BUTTERFLY_SIZE);
			if (butterflyExited)
			{
				butterflyState.active = false;
				butterflyState.respawnFrames = CalcButterflyRespawnFrames(elapsedSeconds, progress) + (int)i * 8;
			}
		}
		else
		{
			if (butterflyState.respawnFrames > 0)
			{
				butterflyState.respawnFrames--;
			}
			else
			{
				SpawnButterfly(butterflyState, minuteStage, progress, false, (int)i * 8);
			}
		}
	}

	if (player)
	{
		SDL_Rect playerRect = player->GetHitbox();
		bool playerAirborne = playerRect.y < (SCREEN_HEIGHT - GROUND_HEIGHT - SPRITE_SIZE - 4);
		if (playerAirborne)
		{
			for (ButterflyState& butterflyState : butterflies)
			{
				if (!butterflyState.active)
				{
					continue;
				}

				SDL_Rect butterflyRect = { (int)butterflyState.x + 24, (int)butterflyState.y + 22, 102, 98 };
				if (SDL_HasIntersection(&playerRect, &butterflyRect))
				{
					butterflyState.active = false;
					butterflyState.respawnFrames = CalcButterflyRespawnFrames(elapsedSeconds, progress);
					butterflyFxFrames = 24;
					butterflyFxX = butterflyRect.x + butterflyRect.w / 2;
					butterflyFxY = butterflyRect.y + butterflyRect.h / 2;
					butterflyCapturedCount++;
					PlayButterflyCaptureSfx();
					butterflyWinFrames = 20;
					comboChain++;
					if (comboChain > 5) comboChain = 5;
					int capturePoints = 200 * comboChain;
					scoreCounter += capturePoints;
					floatingScoreValue = capturePoints;
					floatingScoreFrames = 40;
					floatingScoreX = butterflyRect.x + butterflyRect.w / 2;
					floatingScoreY = butterflyRect.y - 12;
					freezeFrames = 2;
					if (butterflyCapturedCount >= butterflyGoal)
					{
						if (levelNumber >= MAX_LEVELS)
						{
							gameCompleted = true;
							levelCleared = false;
							levelClearFrames = 0;
							levelRestartFrames = 0;
							musicTrackMode = 1;
							musicPatternIndex = 0;
							musicSamplesLeft = 0;
							musicPhase = 0.0;
							PlayWinSfx();
							BeginHighScoreEntry();
						}
						else
						{
							levelCleared = true;
							levelClearFrames = 180;
							levelRestartFrames = 0;
							butterflyWinFrames = 60;
							musicTrackMode = 1;
							musicPatternIndex = 0;
							musicSamplesLeft = 0;
							musicPhase = 0.0;
							PlayWinSfx();
						}
					}
					break;
				}
			}
		}
	}

	if (butterflyFxFrames > 0) butterflyFxFrames--;
	if (stompImpactFrames > 0) stompImpactFrames--;
	if (butterflyWinFrames > 0) butterflyWinFrames--;
	if (floatingScoreFrames > 0)
	{
		floatingScoreFrames--;
		if ((floatingScoreFrames % 3) == 0) floatingScoreY--;
	}
}

void Game::render()
{
	if (splashActive)
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		if (splashTex)
		{
			SDL_Rect splashDst = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
			SDL_RenderCopy(renderer, splashTex, nullptr, &splashDst);
		}
		else
		{
			SDL_Rect splashFallback = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
			SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
			SDL_RenderFillRect(renderer, &splashFallback);
		}

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		const char* splashPrompt = "PRESS ANY KEY TO START";
		const int splashScale = 2;
		const int splashPromptWidth = (int)std::strlen(splashPrompt) * 6 * splashScale;
		const int splashPromptHeight = 7 * splashScale;
		const int splashPromptX = (SCREEN_WIDTH - splashPromptWidth) / 2;
		const int splashPromptY = (SCREEN_HEIGHT - splashPromptHeight) / 2;
		char bestText[48];
		std::snprintf(bestText, sizeof(bestText), "BEST %s %06d", highScoreName.c_str(), highScoreValue);
		const int bestScale = 2;
		const int bestWidth = (int)std::strlen(bestText) * 6 * bestScale;
		DrawText(renderer, splashPromptX, splashPromptY, splashScale, splashPrompt);
		DrawText(renderer, (SCREEN_WIDTH - bestWidth) / 2, splashPromptY + 42, bestScale, bestText);
		SDL_RenderPresent(renderer);
		return;
	}

	int shakeX = 0;
	int shakeY = 0;
	if (shakeFrames > 0 || hitDangerFrames > 0)
	{
		if (shakeFrames > 0)
		{
			shakeFrames--;
		}

		int amp = (hitDangerFrames > 0) ? 8 : 2;
		shakeX = ((SDL_GetTicks() / 11) % (amp * 2 + 1)) - amp;
		shakeY = ((SDL_GetTicks() / 13) % (amp * 2 + 1)) - amp;
	}

	if (currentTimeLeft <= 30)
	{
		Uint32 pulse = (SDL_GetTicks() / 120) % 3;
		if (pulse == 0) SDL_SetRenderDrawColor(renderer, 120, 12, 12, 255);
		else if (pulse == 1) SDL_SetRenderDrawColor(renderer, 80, 8, 8, 255);
		else SDL_SetRenderDrawColor(renderer, 18, 6, 6, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 107, 185, 240, 255);
	}
	SDL_RenderClear(renderer);

	int skyOffset = (int)skyParallaxX;
	// DrawSun(renderer, 762 + skyOffset + shakeX, 72 + shakeY, 6);
	// DrawRainbow(renderer, 704 + skyOffset + shakeX, 126 + shakeY, 3);

	int cx = (int)cloudParallaxX;
	DrawCloud(renderer, 90 + cx + shakeX, 86 + shakeY, 6);
	DrawCloud(renderer, 350 + cx + shakeX, 118 + shakeY, 5);
	DrawCloud(renderer, 690 + cx + shakeX, 74 + shakeY, 6);
	DrawCloud(renderer, 90 + cx + SCREEN_WIDTH + shakeX, 86 + shakeY, 6);
	DrawCloud(renderer, 350 + cx + SCREEN_WIDTH + shakeX, 118 + shakeY, 5);
	DrawCloud(renderer, 690 + cx + SCREEN_WIDTH + shakeX, 74 + shakeY, 6);
	DrawCloud(renderer, 90 + cx - SCREEN_WIDTH + shakeX, 86 + shakeY, 6);
	DrawCloud(renderer, 350 + cx - SCREEN_WIDTH + shakeX, 118 + shakeY, 5);
	DrawCloud(renderer, 690 + cx - SCREEN_WIDTH + shakeX, 74 + shakeY, 6);

	const int groundTopY = SCREEN_HEIGHT - GROUND_HEIGHT;
	int tx = (int)treeParallaxX;
	DrawTree(renderer, 130 + tx + shakeX, groundTopY + shakeY, 8);
	DrawTree(renderer, 420 + tx + shakeX, groundTopY + shakeY, 7);
	DrawTree(renderer, 790 + tx + shakeX, groundTopY + shakeY, 9);
	DrawTree(renderer, 130 + tx + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 8);
	DrawTree(renderer, 420 + tx + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 7);
	DrawTree(renderer, 790 + tx + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 9);
	DrawTree(renderer, 130 + tx - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 8);
	DrawTree(renderer, 420 + tx - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 7);
	DrawTree(renderer, 790 + tx - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 9);

	int flowerOffset = (int)flowerParallaxX;
	DrawFlowerCluster(renderer, 36 + flowerOffset + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 236 + flowerOffset + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 472 + flowerOffset + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 708 + flowerOffset + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 36 + flowerOffset + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 236 + flowerOffset + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 472 + flowerOffset + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 708 + flowerOffset + SCREEN_WIDTH + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 36 + flowerOffset - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 236 + flowerOffset - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 472 + flowerOffset - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 5);
	DrawFlowerCluster(renderer, 708 + flowerOffset - SCREEN_WIDTH + shakeX, groundTopY + shakeY, 5);

	if (butterflyTex)
	{
		for (const ButterflyState& butterflyState : butterflies)
		{
			if (!butterflyState.active)
			{
				continue;
			}
			SDL_Rect src = { butterflyState.frame * BUTTERFLY_SIZE, 0, BUTTERFLY_SIZE, BUTTERFLY_SIZE };
			SDL_Rect dst = { (int)butterflyState.x + shakeX, (int)butterflyState.y + shakeY, BUTTERFLY_SIZE, BUTTERFLY_SIZE };
			SDL_RenderCopy(renderer, butterflyTex, &src, &dst);
		}
	}

	if (butterflyFxFrames > 0)
	{
		DrawButterflyFx(renderer, butterflyFxX + shakeX, butterflyFxY + shakeY, butterflyFxFrames);
	}

	if (stompImpactFrames > 0)
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 255, 248, 112, 120);
		SDL_Rect stompFlash = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderFillRect(renderer, &stompFlash);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 70);
		SDL_Rect stompShade = { 0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT };
		SDL_RenderFillRect(renderer, &stompShade);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	}

	SDL_Rect ground = { 0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, GROUND_HEIGHT };
	SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
	SDL_RenderFillRect(renderer, &ground);
	SDL_Rect dirt = { 0, SCREEN_HEIGHT - GROUND_HEIGHT, SCREEN_WIDTH, 12 };
	SDL_SetRenderDrawColor(renderer, 101, 67, 33, 255);
	SDL_RenderFillRect(renderer, &dirt);

	SDL_Rect hudBar = { 0, 0, SCREEN_WIDTH, 56 };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(renderer, &hudBar);

	char scoreText[32];
	char timeText[32];
	char butterflyText[32];
	int timeLeft = currentTimeLeft;
	int timeMinutes = timeLeft / 60;
	int timeSeconds = timeLeft % 60;
	std::snprintf(scoreText, sizeof(scoreText), "SCORE %06d", scoreCounter);
	std::snprintf(timeText, sizeof(timeText), "TIME %02d:%02d", timeMinutes, timeSeconds);
	std::snprintf(butterflyText, sizeof(butterflyText), "X %02d OF %02d", butterflyCapturedCount, butterflyGoal);

	SDL_SetRenderDrawColor(renderer, 58, 94, 161, 255);
	DrawText(renderer, 26, 18, 4, scoreText);
	DrawText(renderer, SCREEN_WIDTH - 24 - (int)std::strlen(timeText) * 24 + 2, 18, 4, timeText);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	DrawText(renderer, 24, 16, 4, scoreText);
	if (timeLeft <= 30 && ((SDL_GetTicks() / 200) % 2 == 0))
	{
		SDL_SetRenderDrawColor(renderer, 255, 84, 84, 255);
	}
	else
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	}
	DrawText(renderer, SCREEN_WIDTH - 24 - (int)std::strlen(timeText) * 24, 16, 4, timeText);

	const int butterflyIconWidth = 24;
	const int butterflyTextWidth = (int)std::strlen(butterflyText) * 24;
	const int butterflyBlockWidth = butterflyIconWidth + 12 + butterflyTextWidth;
	const int butterflyStartX = SCREEN_WIDTH / 2 - butterflyBlockWidth / 2;
	SDL_SetRenderDrawColor(renderer, 58, 94, 161, 255);
	DrawButterflyIcon(renderer, butterflyStartX + 2, 18, 4);
	DrawText(renderer, butterflyStartX + butterflyIconWidth + 14 + 2, 18, 4, butterflyText);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	DrawButterflyIcon(renderer, butterflyStartX, 16, 4);
	DrawText(renderer, butterflyStartX + butterflyIconWidth + 14, 16, 4, butterflyText);

	if (floatingScoreFrames > 0)
	{
		char floatText[24];
		std::snprintf(floatText, sizeof(floatText), "%d", floatingScoreValue);
		SDL_SetRenderDrawColor(renderer, 58, 94, 161, 255);
		DrawText(renderer, floatingScoreX + 1, floatingScoreY + 1, 3, floatText);
		SDL_SetRenderDrawColor(renderer, 255, 252, 112, 255);
		DrawText(renderer, floatingScoreX, floatingScoreY, 3, floatText);
	}

	if (player)
	{
		if (playerFlickerFrames == 0 || ((playerFlickerFrames / 4) % 2 == 0))
		{
			player->Render();
		}
	}

	for (EnemyState& enemyState : enemies)
	{
		if (!enemyState.obj || !enemyState.active)
		{
			continue;
		}
		if (enemyState.flickerFrames == 0 || ((enemyState.flickerFrames / 4) % 2 == 0))
		{
			enemyState.obj->Render();
		}
	}

	if (hitDangerFrames > 0)
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (((hitDangerFrames / 2) % 2) == 0)
		{
			SDL_SetRenderDrawColor(renderer, 220, 24, 24, 120);
		}
		else
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
		}
		SDL_Rect dangerOverlay = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderFillRect(renderer, &dangerOverlay);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	}

	if (isPaused)
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
		SDL_Rect pauseShade = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderFillRect(renderer, &pauseShade);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_Rect bar1 = { SCREEN_WIDTH / 2 - 20, SCREEN_HEIGHT / 2 - 34, 14, 68 };
		SDL_Rect bar2 = { SCREEN_WIDTH / 2 + 6, SCREEN_HEIGHT / 2 - 34, 14, 68 };
		SDL_RenderFillRect(renderer, &bar1);
		SDL_RenderFillRect(renderer, &bar2);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	}

	if (levelCleared)
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 255, 140, 190, 170);
		SDL_Rect clearShade = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderFillRect(renderer, &clearShade);
		if (butterflyWinFrames > 0)
		{
			SDL_SetRenderDrawColor(renderer, 255, 220, 235, 90);
			SDL_RenderFillRect(renderer, &clearShade);
		}
		SDL_SetRenderDrawColor(renderer, 255, 248, 96, 255);
		DrawText(renderer, SCREEN_WIDTH / 2 - 156, SCREEN_HEIGHT / 2 - 40, 5, "LEVEL CLEAR");
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		DrawText(renderer, SCREEN_WIDTH / 2 - 168, SCREEN_HEIGHT / 2 + 16, 4, "NEXT STAGE LOADING");
		DrawText(renderer, SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 58, 3, "PRESS ENTER TO PAUSE");
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	}
	else if (gameCompleted)
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 190);
		SDL_Rect completeShade = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderFillRect(renderer, &completeShade);
		SDL_SetRenderDrawColor(renderer, 255, 220, 96, 255);
		DrawText(renderer, SCREEN_WIDTH / 2 - 198, SCREEN_HEIGHT / 2 - 72, 4, "ALL STAGES CLEAR");
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		char bestText[48];
		std::snprintf(bestText, sizeof(bestText), "BEST %s %06d", highScoreName.c_str(), highScoreValue);
		DrawText(renderer, SCREEN_WIDTH / 2 - ((int)std::strlen(bestText) * 9), SCREEN_HEIGHT / 2 - 14, 3, bestText);

		if (nameEntryActive)
		{
			std::string entryText = nameEntryBuffer;
			if ((SDL_GetTicks() / 250) % 2 == 0 && (int)entryText.size() < MAX_NAME_LENGTH)
			{
				entryText.push_back('_');
			}
			DrawText(renderer, SCREEN_WIDTH / 2 - 132, SCREEN_HEIGHT / 2 + 28, 3, "NEW HIGH SCORE ENTER NAME");
			DrawText(renderer, SCREEN_WIDTH / 2 - ((int)entryText.size() * 9), SCREEN_HEIGHT / 2 + 68, 3, entryText.c_str());
		}
		else
		{
			DrawText(renderer, SCREEN_WIDTH / 2 - 162, SCREEN_HEIGHT / 2 + 52, 3, "PRESS ENTER TO RESTART");
		}
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	}
	else if (gameOver)
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 20, 0, 0, 190);
		SDL_Rect gameOverShade = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		SDL_RenderFillRect(renderer, &gameOverShade);
		SDL_SetRenderDrawColor(renderer, 255, 84, 84, 255);
		DrawText(renderer, SCREEN_WIDTH / 2 - 132, SCREEN_HEIGHT / 2 - 52, 5, "GAME OVER");
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		DrawText(renderer, SCREEN_WIDTH / 2 - 168, SCREEN_HEIGHT / 2 + 4, 4, "TIME UP TRY AGAIN");

		char bestText[48];
		std::snprintf(bestText, sizeof(bestText), "BEST %s %06d", highScoreName.c_str(), highScoreValue);
		DrawText(renderer, SCREEN_WIDTH / 2 - ((int)std::strlen(bestText) * 9), SCREEN_HEIGHT / 2 + 52, 3, bestText);

		if (nameEntryActive)
		{
			std::string entryText = nameEntryBuffer;
			if ((SDL_GetTicks() / 250) % 2 == 0 && (int)entryText.size() < MAX_NAME_LENGTH)
			{
				entryText.push_back('_');
			}
			DrawText(renderer, SCREEN_WIDTH / 2 - 132, SCREEN_HEIGHT / 2 + 90, 3, "NEW HIGH SCORE ENTER NAME");
			DrawText(renderer, SCREEN_WIDTH / 2 - ((int)entryText.size() * 9), SCREEN_HEIGHT / 2 + 128, 3, entryText.c_str());
		}
		else
		{
			DrawText(renderer, SCREEN_WIDTH / 2 - 162, SCREEN_HEIGHT / 2 + 100, 3, "PRESS ENTER TO RESTART");
		}
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	}

	SDL_RenderPresent(renderer);
}

void Game::clean()
{
	if (player)
	{
		delete player;
		player = nullptr;
	}

	for (EnemyState& enemyState : enemies)
	{
		if (enemyState.obj)
		{
			delete enemyState.obj;
			enemyState.obj = nullptr;
		}
	}
	enemies.clear();

	if (butterflyTex)
	{
		SDL_DestroyTexture(butterflyTex);
		butterflyTex = nullptr;
	}

	if (splashTex)
	{
		SDL_DestroyTexture(splashTex);
		splashTex = nullptr;
	}

	if (renderer)
	{
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}

	if (window)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	if (audioDevice != 0)
	{
		SDL_CloseAudioDevice(audioDevice);
		audioDevice = 0;
	}

	SDL_StopTextInput();

	IMG_Quit();
	SDL_Quit();
}
