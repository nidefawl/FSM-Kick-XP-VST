#include <math.h>
#include <algorithm>
#include "plugin.h"

using namespace std;

SynthParameter const paraStartFrq =
{
	"StartFrq",
	"Start frequency",
	1,
	240,
	99
};

SynthParameter const paraEndFrq =
{
	"EndFrq",
	"End frequency",
	1,
	240,
	66
};

SynthParameter const paraBuzzAmt =
{
	"Buzz",
	"Amount of Buzz",
	0,
	100,
	0
};

SynthParameter const paraClickAmt =
{
	"Click",
	"Amount of Click",
	0,
	100,
	0
};

SynthParameter const paraPunchAmt =
{
	"Punch",
	"Amount of Punch",
	0,
	100,
	17
};

SynthParameter const paraToneDecay =
{
	"ToneDecR",
	"Tone decay rate",
	1,
	240,
	104,
};

SynthParameter const paraToneShape =
{
	"ToneDecS",
	"Tone decay shape",
	1,
	240,
	17
};

SynthParameter const paraBDecay =
{
	"BuzzDecR",
	"Buzz decay rate",
	1,
	240,
	69
};

SynthParameter const paraCDecay =
{
	"C+P DecR",
	"Click+Punch decay rate",
	1,
	240,
	164
};

SynthParameter const paraDecSlope =
{
	"DecSlope",
	"Amplitude decay slope",
	1,
	240,
	94
};

SynthParameter const paraDecTime =
{
	"DecTime",
	"Amplitude decay time",
	1,
	240,
	41
};

SynthParameter const paraRelSlope =
{
	"RelSlope",
	"Amplitude release slope",
	1,
	240,
	139
};

SynthParameter const *synthParameters[] =
{
	&paraStartFrq,
	&paraEndFrq,
	&paraBuzzAmt,
	&paraClickAmt,
	&paraPunchAmt,
	&paraToneDecay,
	&paraToneShape,
	&paraBDecay,
	&paraCDecay,
	&paraDecSlope,
	&paraDecTime,
	&paraRelSlope,
};


float to_range(float floatVal, const SynthParameter &param) {
	float fByte = (float)param.MinValue + floatVal*(float)(param.MaxValue - param.MinValue);
	return fByte < param.MinValue ? param.MinValue : fByte > param.MaxValue ? param.MaxValue : fByte;
}
float from_range(byte byteVal, const SynthParameter &param) {
	float fByte = byteVal;
	fByte = fByte - param.MinValue;
	fByte = fByte / (float)(param.MaxValue - param.MinValue);
	return fByte < 0.0f ? 0.0f : fByte > 1.0f ? 1.0f : fByte;
}

typedef unsigned long int  u4;
typedef struct ranctx { u4 a; u4 b; u4 c; u4 d; } ranctx;

#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
u4 ranval(ranctx *x) {
	u4 e = x->a - rot(x->b, 27);
	x->a = x->b ^ rot(x->c, 17);
	x->b = x->c + x->d;
	x->c = x->d + e;
	x->d = e + x->a;
	return x->d;
}

void raninit(ranctx *x, u4 seed) {
	u4 i;
	x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
	for (i = 0; i<20; ++i) {
		(void)ranval(x);
	}
}
static ranctx rctx;
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new FSM_VST_Plugin (audioMaster);
}

void FSM_VST_Program::set(char* _name, int start, int end, 
	int buzz, int click, int punch, int tDecR, int tDecS, 
	int bDecR, int CPDecR, int ADecS, int ADecT, int ARelS) {
	vst_strncpy(this->name, _name, kVstMaxProgNameLen);
	bStartFrq = from_range(start, paraStartFrq);
	bEndFrq = from_range(end, paraEndFrq);
	bBuzzAmt = from_range(buzz, paraBuzzAmt);
	bClickAmt = from_range(click, paraClickAmt);
	bPunchAmt = from_range(punch, paraPunchAmt);

	bToneDecay = from_range(tDecR, paraToneDecay);
	bToneShape = from_range(tDecS, paraToneShape);

	bBDecay = from_range(bDecR, paraBDecay);
	bCDecay = from_range(CPDecR, paraCDecay);
	bDecSlope = from_range(ADecS, paraDecSlope);
	bDecTime = from_range(ADecT, paraDecTime);
	bRelSlope = from_range(ARelS, paraRelSlope);
}
#define RND(x) (ranval(&rctx)%x)
FSM_VST_Plugin::FSM_VST_Plugin (audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
	raninit(&rctx, 238947);
	lock = new Lock();
	programs = new FSM_VST_Program[kNumPrograms];
	fVolume = 0.8f;
	issetprogram = false;
#ifdef DEBUG_CONSOLE
	hout = NULL;
#endif // DEBUG_CONSOLE
	// initialize programs
	programs[0].set("Init", 145, 50, 55, 28, 47, 30, 27, 55, 55, 1, 32, 105);
	programs[1].set("Soft", 99, 66,0, 0, 17, 104, 17, 69, 164, 94, 41, 139);
	for (int i = 2; i < kNumPrograms; i++) {
		char presetname[16];
		sprintf_s(presetname, kVstMaxProgNameLen, "Random %i", i);
		programs[i].set(presetname, 
			64 + RND(64),
			RND(96),
			RND(30),
			RND(20),
			RND(50),
			1 + RND(240),
			1 + RND(240),
			1 + RND(240),
			1 + RND(240),
			1 + RND(240),
			1 + RND(240),
			1 + RND(240));
	}
	setProgram(0);

	if (audioMaster)
	{
		setNumInputs (0);				// no inputs
		setNumOutputs (kNumOutputs);	// 2 outputs, 1 for each oscillator
		canProcessReplacing ();
		isSynth ();
		setUniqueID ('FSMX');			// <<<! *must* change this!!!!
	}
#ifdef DEBUG_CONSOLE
	startConsoleWin(300, 300, "Debug");
#endif //DEBUG_CONSOLE

	initProcess ();
	suspend ();
}

void FSM_VST_Plugin::initProcess()
{
	for (int i = 0; i<1024; i++)
		thumpdata1[i] = float(sin(1.37*i + 0.1337*(1024 - i)*sin(1.1*i))*pow(1.0 / 256.0, i / 1024.0));
}

FSM_VST_Plugin::~FSM_VST_Plugin ()
{
	if (lock) {
		delete lock;
	}
	if (programs)
		delete[] programs;
#ifdef DEBUG_CONSOLE
	FreeConsole();
#endif;
}

void FSM_VST_Plugin::setProgram (VstInt32 program)
{
	if (program < 0 || program >= kNumPrograms)
		return;
	lock->lock();
	allNotesOff(false);
	curProgram = program;
	lock->unlock();
}

void FSM_VST_Plugin::setProgramName (char* name)
{
	//if (name != NULL && programs != NULL  && curProgram >= 0 && curProgram < kNumPrograms && programs[curProgram].name != NULL)
	//	vst_strncpy (programs[curProgram].name, name, kVstMaxProgNameLen); 
	//if (name != NULL) {

	//	//dprintf("setProgramName %s\n", name);
	//}
}

void FSM_VST_Plugin::getProgramName (char* name)
{
	if (name != NULL && programs  != NULL  && curProgram >= 0  && programs[curProgram].name != NULL )
		vst_strncpy (name, programs[curProgram].name, kVstMaxProgNameLen);
}

void FSM_VST_Plugin::getParameterLabel (VstInt32 index, char* label)
{
	vst_strncpy(label, "", kVstMaxParamStrLen);
}

void FSM_VST_Plugin::getParameterDisplay (VstInt32 index, char* text)
{
	text[0] = 0;
	switch (index)
	{
	case kVolume:		float2string(fVolume, text, kVstMaxParamStrLen);	break;
	case kStartFrq: float2string(to_range(current()->bStartFrq, paraStartFrq), text, kVstMaxParamStrLen); break;
	case kEndFrq: float2string(to_range(current()->bEndFrq, paraEndFrq), text, kVstMaxParamStrLen); break;
	case kBuzzAmt: float2string(to_range(current()->bBuzzAmt, paraBuzzAmt), text, kVstMaxParamStrLen);  break;
	case kClickAmt: float2string(to_range(current()->bClickAmt, paraClickAmt), text, kVstMaxParamStrLen); break;
	case kPunchAmt: float2string(to_range(current()->bPunchAmt, paraPunchAmt), text, kVstMaxParamStrLen); break;
	case kToneDecay: float2string(to_range(current()->bToneDecay, paraToneDecay), text, kVstMaxParamStrLen); break;
	case kToneShape: float2string(to_range(current()->bToneShape, paraToneShape), text, kVstMaxParamStrLen); break;
	case kBDecay: float2string(to_range(current()->bBDecay, paraBDecay), text, kVstMaxParamStrLen); break;
	case kCDecay: float2string(to_range(current()->bCDecay, paraCDecay), text, kVstMaxParamStrLen); break;
	case kDecSlope: float2string(to_range(current()->bDecSlope, paraDecSlope), text, kVstMaxParamStrLen); break;
	case kDecTime: float2string(to_range(current()->bDecTime, paraDecTime), text, kVstMaxParamStrLen);  break;
	case kRelSlope: float2string(to_range(current()->bRelSlope, paraRelSlope), text, kVstMaxParamStrLen); break;
	}
}

void FSM_VST_Plugin::getParameterName (VstInt32 index, char* label)
{
	switch (index)
	{
	case kVolume:		vst_strncpy(label, "Volume", kVstMaxParamStrLen);	return;
	}
	
	if (index > 0 && index < kNumParams) {
		const SynthParameter* param = synthParameters[index-1];
		vst_strncpy(label, param->Name, kVstMaxParamStrLen);
	}
}

void FSM_VST_Plugin::setParameter (VstInt32 index, float value)
{
	FSM_VST_Program *ap = &programs[curProgram];
	switch (index)
	{
	case kVolume:		fVolume = value;	break;
	case kStartFrq: current()->bStartFrq = value; break;
	case kEndFrq: current()->bEndFrq = value; break;
	case kBuzzAmt: current()->bBuzzAmt = value; break;
	case kClickAmt: current()->bClickAmt = value; break;
	case kPunchAmt: current()->bPunchAmt = value; break;
	case kToneDecay: current()->bToneDecay = value; break;
	case kToneShape: current()->bToneShape = value; break;
	case kBDecay: current()->bBDecay = value; break;
	case kCDecay: current()->bCDecay = value; break;
	case kDecSlope: current()->bDecSlope = value; break;
	case kDecTime: current()->bDecTime = value; break;
	case kRelSlope: current()->bRelSlope = value; break;
	}
}

float FSM_VST_Plugin::getParameter (VstInt32 index)
{
	float value = 0;
	switch (index)
	{
	case kVolume:		value = fVolume;	break;
	case kStartFrq: value = current()->bStartFrq; break;
	case kEndFrq: value = current()->bEndFrq; break;
	case kBuzzAmt: value = current()->bBuzzAmt; break;
	case kClickAmt: value = current()->bClickAmt; break;
	case kPunchAmt: value = current()->bPunchAmt; break;
	case kToneDecay: value = current()->bToneDecay; break;
	case kToneShape: value = current()->bToneShape; break;
	case kBDecay: value = current()->bBDecay; break;
	case kCDecay: value = current()->bCDecay; break;
	case kDecSlope: value = current()->bDecSlope; break;
	case kDecTime: value = current()->bDecTime; break;
	case kRelSlope: value = current()->bRelSlope; break;
	}
	return value;
}

void FSM_Voice::setParameters(ProgramParameters *ptval, float srate)
{
	this->StartFrq = (float)(33.0*pow(128, to_range(ptval->bStartFrq, paraStartFrq) / 240.0));
	this->EndFrq = (float)(33.0*pow(16, to_range(ptval->bEndFrq, paraEndFrq) / 240.0));
	this->TDecay = (float)((to_range(ptval->bToneDecay, paraToneDecay) / 240.0)*(1.0 / 400.0)*(44100.0 / srate));
	this->TShape = (float)(to_range(ptval->bToneShape, paraToneShape) / 240.0);
	this->DSlope = (float)pow(20, to_range(ptval->bDecSlope, paraDecSlope) / 240.0 - 1) * 25 / srate;
	this->DTime = (float)(to_range(ptval->bDecTime, paraDecTime)*srate / 240.0);
	this->RSlope = (float)pow(20, to_range(ptval->bRelSlope, paraRelSlope) / 240.0 - 1) * 25 / srate;
	this->BDecay = (float)(to_range(ptval->bBDecay, paraBDecay) / 240.0);
	this->CDecay = (float)(to_range(ptval->bCDecay, paraCDecay) / 240.0);
	this->ClickAmt = (float)(to_range(ptval->bClickAmt, paraClickAmt) / 100.0);
	this->BuzzAmt = 3 * (float)(to_range(ptval->bBuzzAmt, paraBuzzAmt) / 100.0);
	this->PunchAmt = (float)(to_range(ptval->bPunchAmt, paraPunchAmt) / 100.0);
	if (this->currentNote != NOTE_OFF)
	{
		int v = this->currentNote-24;
		this->PitchLimit = (float)(440.0*pow(2, (v - 69) / 12.0));
	}
}

bool FSM_VST_Plugin::getOutputProperties (VstInt32 index, VstPinProperties* properties)
{
	if (index == 0 || index == 1)
	{
		properties->flags = kVstPinIsActive | kVstPinIsStereo;
	}
	if (index == 0)
	{
		strcpy(properties->label,	   "Left output");
		strcpy(properties->shortLabel, "L out");
		return true;
	}
	else if (index == 1)
	{
		strcpy(properties->label,	   "Right output");
		strcpy(properties->shortLabel, "R out");
		return true;
	}
	return false;
}

bool FSM_VST_Plugin::getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text)
{
	if (index >= 0 && index < kNumPrograms)
	{
		vst_strncpy (text, programs[index].name, kVstMaxProgNameLen);
		return true;
	}
	return false;
}

bool FSM_VST_Plugin::getEffectName (char* name)
{
	vst_strncpy(name, "DrumSynth", kVstMaxEffectNameLen);
	return true;
}

bool FSM_VST_Plugin::getVendorString (char* text)
{
	vst_strncpy(text, PLUGIN_VENDOR_NAME, kVstMaxVendorStrLen);
	return true;
}

bool FSM_VST_Plugin::getProductString (char* text)
{
	vst_strncpy(text, PLUGIN_PRODUCT_NAME, kVstMaxProductStrLen);
	return true;
}

VstInt32 FSM_VST_Plugin::getVendorVersion ()
{ 
	return 1000; 
}

VstInt32 FSM_VST_Plugin::canDo (char* text)
{
	if (!strcmp (text, "receiveVstEvents"))
		return 1;
	if (!strcmp(text, "receiveVstMidiEvent"))
		return 1;
	if (!strcmp(text, "receiveVstTimeInfo"))
		return 1;
	return -1;	// explicitly can't do; 0 => don't know
}

VstInt32 FSM_VST_Plugin::getNumMidiInputChannels ()
{
	return 1; // we are monophonic
}

VstInt32 FSM_VST_Plugin::getNumMidiOutputChannels ()
{
	return 0; // no MIDI output back to Host app
}

void FSM_VST_Plugin::setSampleRate (float sampleRate)
{
	lock->lock();
	allNotesOff(true);
	AudioEffectX::setSampleRate(sampleRate);
	lock->unlock();
}

void FSM_VST_Plugin::setBlockSize (VstInt32 blockSize)
{
	lock->lock();
	allNotesOff(true);
	AudioEffectX::setBlockSize(blockSize);
	lock->unlock();
}


#define ENTER_LOCK \
	if (!locked) { \
		locked++; \
		lock->lock(); \
	}

VstInt32 FSM_VST_Plugin::processEvents(VstEvents* ev)
{
	int locked = 0;
	for (VstInt32 i = 0; i < ev->numEvents; i++)
	{
		if ((ev->events[i])->type != kVstMidiType)
			continue;

		VstMidiEvent* event = (VstMidiEvent*)ev->events[i];
		char* midiData = event->midiData;
		VstInt32 status = midiData[0] & 0xf0;	// ignoring channel
		if (status == 0x90 || status == 0x80)	// we only look at notes
		{
			VstInt32 note = midiData[1] & 0x7f;
			VstInt32 velocity = midiData[2] & 0x7f;
			if (status == 0x80)
				velocity = 0;	// note off by velocity 0
			if (!velocity) {
				ENTER_LOCK;
				noteOff(note);
			}
			else {
				ENTER_LOCK;
				noteOn(note, velocity, event->deltaFrames);
			}
		}
		else if (status == 0xb0)
		{
			if (midiData[1] == 0x7e || midiData[1] == 0x7b) {	// all notes off
				ENTER_LOCK;
				allNotesOff(true);
			}
		}
		event++;
	}
	if (locked) {
		lock->unlock();
	}
	return 1;
}

void FSM_VST_Plugin::noteOff(VstInt32 note) {

	std::vector<FSM_Voice*>::iterator it = voices.begin();
	while (it != voices.end())
	{
		FSM_Voice *voice = *it;
		if (voice->currentNote == note) {
			voice->release(true);
		}
		it++;
	}
}

void FSM_VST_Plugin::noteOn(VstInt32 note, VstInt32 velocity, VstInt32 delta) {
	if (issetprogram)
		return;
	FSM_Voice* voice;
	voice = new FSM_Voice(note, velocity, delta);
	voice->setParameters(current(), sampleRate);
	voice->trigger();
	voices.push_back(voice);
}

void FSM_VST_Plugin::allNotesOff(bool decay) {
	std::vector<FSM_Voice*>::iterator it = voices.begin();
	while (it != voices.end())
	{
		FSM_Voice *_note = *it;
		_note->release(decay);
		it++;
	}
}

#define INTERPOLATE(pos,start,end) ((start)+(pos)*((end)-(start)))

bool FSM_VST_Plugin::processVoice(FSM_Voice *trk, float *pout, int c, float gain)
{

	trk->OscPhase = fmod(trk->OscPhase, 1.0);
	float Ratio = trk->ThisEndFrq / trk->ThisStartFrq;
	int i = 0;
	double xSin = trk->xSin, xCos = trk->xCos;
	double dxSin = trk->dxSin, dxCos = trk->dxCos;
	float LVal = 0;
	float Amp = trk->Amp;
	float DecAmp = trk->DecAmp;
	float BAmp = trk->BAmp;
	float MulBAmp = trk->MulBAmp;
	float CAmp = trk->CAmp;
	float MulCAmp = trk->MulCAmp;
	float Vol = 0.5f*trk->ThisCurVolume*gain;
	bool amphigh = Amp >= 16;
	int Age = trk->Age;
	float sr = this->getSampleRate();
	float odsr = 1.0f / sr;
	int proc = 0;
	while (i<c)
	{
		if (trk->LeftOver <= 0)
		{
			trk->LeftOver = 32;
			double EnvPoint = trk->EnvPhase*trk->ThisTDecay;
			double ShapedPoint = pow(EnvPoint, trk->ThisTShape*2.0);
			trk->Frequency = (float)(trk->ThisStartFrq*pow((double)Ratio, ShapedPoint));
			if (trk->Frequency>10000.f) trk->EnvPhase = 6553600;
			if (trk->EnvPhase<trk->ThisDTime)
			{
				trk->DecAmp = DecAmp = trk->ThisDSlope;
				trk->Amp = Amp = (float)(1 - DecAmp*trk->EnvPhase);
			}
			else
			{
				DecAmp = trk->ThisDSlope;
				Amp = (float)(1 - DecAmp*trk->ThisDTime);
				if (Amp>0)
				{
					trk->DecAmp = DecAmp = trk->ThisRSlope;
					trk->Amp = Amp = Amp - DecAmp*(trk->EnvPhase - trk->ThisDTime);
				}
			}
			if (trk->Amp <= 0)
			{
				trk->Amp = 0;
				trk->DecAmp = 0;
				if (!trk->SamplesToGo)
					return amphigh;
			}

			trk->BAmp = BAmp = trk->BuzzAmt*(float)(pow(1.0f / 256.0f, trk->ThisBDecay*trk->EnvPhase*(odsr * 10)));
			float CVal = (float)(pow(1.0f / 256.0f, trk->ThisCDecay*trk->EnvPhase*(odsr * 20)));
			trk->CAmp = CAmp = trk->ClickAmt*CVal;
			trk->Frequency *= (1 + 2 * trk->PunchAmt*CVal*CVal*CVal);
			if (trk->Frequency>10000) trk->Frequency = 10000;
			if (trk->Frequency<trk->ThisPitchLimit) trk->Frequency = trk->ThisPitchLimit;

			trk->MulBAmp = MulBAmp = (float)pow(1.0f / 256.0f, trk->ThisBDecay*(10 * odsr));
			trk->MulCAmp = MulCAmp = (float)pow(1.0f / 256.0f, trk->ThisCDecay*(10 * odsr));
			xSin = (float)sin(2.0*3.141592665*trk->OscPhase);
			xCos = (float)cos(2.0*3.141592665*trk->OscPhase);
			dxSin = (float)sin(2.0*3.141592665*trk->Frequency / sr);
			dxCos = (float)cos(2.0*3.141592665*trk->Frequency / sr);
			LVal = 0.0;
			trk->dxSin = dxSin, trk->dxCos = dxCos;
		}
		int max = min(i + trk->LeftOver, c);
		if (trk->SamplesToGo>0)
			max = min(max, i + trk->SamplesToGo - 1);
		
		if (Amp>0.00001f && Vol>0)
		{
			proc++;
			amphigh = true;
			float OldAmp = Amp;
			if (BAmp>0.01f)
			{
				for (int j = i; j<max; j++)
				{
					pout[j] += float(LVal = float(Amp*Vol*xSin));
					if (xSin>0)
					{
						float D = (float)(Amp*Vol*BAmp*xSin*xCos);
						pout[j] -= D;
						LVal -= D;
					}
					double xSin2 = double(xSin*dxCos + xCos*dxSin);
					double xCos2 = double(xCos*dxCos - xSin*dxSin);
					xSin = xSin2; xCos = xCos2;
					Amp -= DecAmp;
					BAmp *= MulBAmp;
				}
			}
			else
				for (int j = i; j<max; j++)
				{
					pout[j] += float(LVal = float(Amp*Vol*xSin));
					double xSin2 = double(xSin*dxCos + xCos*dxSin);
					double xCos2 = double(xCos*dxCos - xSin*dxSin);
					xSin = xSin2; xCos = xCos2;
					Amp -= DecAmp;
				}
			if (OldAmp>0.1f && CAmp>0.001f)
			{
				int max2 = i + min(max - i, 1024 - Age);
				float LVal2 = 0.f;
				for (int j = i; j<max2; j++)
				{
					pout[j] += (LVal2 = OldAmp*Vol*CAmp*this->thumpdata1[Age]);
					OldAmp -= DecAmp;
					CAmp *= MulCAmp;
					Age++;
				}
				LVal += LVal2;
			}
		}
		if (Amp)
		{
			trk->OscPhase += (max - i)*trk->Frequency / sr;
			trk->EnvPhase += max - i;
			trk->LeftOver -= max - i;
		}
		else
			trk->LeftOver = 32000;
		if (trk->SamplesToGo>0) trk->SamplesToGo -= max - i;
		i = max;
	}
	
	trk->xSin = xSin, trk->xCos = xCos;
	trk->Amp = Amp;
	trk->BAmp = BAmp;
	trk->CAmp = CAmp;
	trk->Age = Age;
	return amphigh;
}

void FSM_VST_Plugin::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
	memset(outputs[0], 0, sampleFrames * sizeof(float));
	memset(outputs[1], 0, sampleFrames * sizeof(float));
	if (issetprogram)
		return;
	if (this->voices.empty()) {
		return;
	}
	float gain = powf(fVolume*SCALE_GAIN_OVERHEAD, 1.F / LOG_SCALE_GAIN);
	lock->lock();
	for (std::vector<FSM_Voice*>::iterator it = this->voices.begin(); it != this->voices.end(); ) {
		FSM_Voice* voice = *it;
		if (voice->killed) {
			delete voice;
			it = voices.erase(it);
			continue;
		}
		float* out1 = outputs[0];
		int samples = sampleFrames;
		if (voice->currentDelta >= sampleFrames)	// future
		{
			voice->currentDelta -= sampleFrames;
			++it;
			continue;
		}
		if (voice->currentDelta > 0)
		{
			out1 += voice->currentDelta;
			samples -= voice->currentDelta;
			voice->currentDelta = 0;
		}
		if (samples > 0) {

			bool amplitudeHigh = this->processVoice(voice, out1, samples, gain);
			if (voice->released && !amplitudeHigh) {
				delete voice;
				it = voices.erase(it);
				continue;
			}
		}
		++it;
	}
	lock->unlock();
	memcpy(outputs[1], outputs[0], sizeof(float)*sampleFrames);
}


FSM_Voice::FSM_Voice(VstInt32 note, VstInt32 velocity, VstInt32 delta) {
	currentNote = note;
	currentDelta = delta;
	currentVelocity = velocity;
	released = false;
	killed = false;
	SamplesToGo = 0;
	LeftOver = 0;
	EnvPhase = 0;
	OscPhase = 0;
	CurVolume = 0;
	Age = 0;
	Amp = 0;
	DecAmp = 0;
	BAmp = 0;
	MulBAmp = 0;
	CAmp = 0;
	MulCAmp = 0;
	Frequency = 0;
	xSin = 0;
	xCos = 0;
	dxSin = 0;
	dxCos = 0;
}

void FSM_Voice::trigger()
{
	this->SamplesToGo = 0;
	this->EnvPhase = 0;
	this->OscPhase = this->ClickAmt;
	this->LeftOver = 0;
	this->Age = 0;
	this->Amp = 32;
	this->CurVolume = this->velocity();
	this->ThisPitchLimit = this->PitchLimit;
	this->ThisDTime = this->DTime;
	this->ThisDSlope = this->DSlope;
	this->ThisRSlope = this->RSlope;
	this->ThisBDecay = this->BDecay;
	this->ThisCDecay = this->CDecay;
	this->ThisTDecay = this->TDecay;
	this->ThisTShape = this->TShape;
	this->ThisStartFrq = this->StartFrq;
	this->ThisEndFrq = this->EndFrq;
	this->ThisCurVolume = this->CurVolume;
}

void FSM_Voice::release(bool decay) {
	released = true;
	killed = !decay;
	if (this->EnvPhase<this->ThisDTime)
	{
		this->ThisDTime = (float) this->EnvPhase;
	}
}

FSM_VST_Program::FSM_VST_Program()
{
	vst_strncpy(name, "Init", kVstMaxProgNameLen);

	bStartFrq = from_range(paraStartFrq.DefValue, paraStartFrq);
	bEndFrq = from_range(paraEndFrq.DefValue, paraEndFrq);
	bBuzzAmt = from_range(paraBuzzAmt.DefValue, paraBuzzAmt);
	bClickAmt = from_range(paraClickAmt.DefValue, paraClickAmt);
	bPunchAmt = from_range(paraPunchAmt.DefValue, paraPunchAmt);

	bToneDecay = from_range(paraToneDecay.DefValue, paraToneDecay);
	bToneShape = from_range(paraToneShape.DefValue, paraToneShape);

	bBDecay = from_range(paraBDecay.DefValue, paraBDecay);
	bCDecay = from_range(paraCDecay.DefValue, paraCDecay);
	bDecSlope = from_range(paraDecSlope.DefValue, paraDecSlope);
	bDecTime = from_range(paraDecTime.DefValue, paraDecTime);
	bRelSlope = from_range(paraRelSlope.DefValue, paraRelSlope);
}
