#include "plugin.h"
#include "Debug.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <conio.h>
#include <fcntl.h>
using namespace std;
#ifdef DEBUG

#ifdef DEBUG_CONSOLE
void FSM_VST_Plugin::startConsoleWin(int width, int height, char* fname)
{


	int hConHandle;
	long lStdHandle;
	FILE *fp;

	// Try to attach to a console
	AllocConsole();
	SetConsoleTitle("Debug Window");
	hout = GetStdHandle(STD_OUTPUT_HANDLE);
	DeleteMenu(GetSystemMenu(GetConsoleWindow(), 0), SC_CLOSE, MF_BYCOMMAND);
	COORD co = {width,height};
	SetConsoleScreenBufferSize(hout, co);
	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
	fprintf(stdout, "Test output to stdout\n");
	fprintf(stderr, "Test output to stderr\n");
}

#endif // DEBUG_CONSOLE

#ifdef DISPATCHER_DEBUG_TRACE

//-----------------------------------------------------------------------------------------------------------------
VstIntPtr FSM_VST_Plugin::dispatcher (VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	std::string debug = "";
	switch (opcode) {
		///< no arguments  @see AudioEffect::open
	case effOpen: debug = "effOpen"; break;		
		///< no arguments  @see AudioEffect::close
	case effClose: debug = "effClose"; break;			
		///< [value]: new program number  @see AudioEffect::setProgram
	case effSetProgram: debug = "effSetProgram"; break;		
		///< [return value]: current program number  @see AudioEffect::getProgram
	case effGetProgram: debug = "effGetProgram"; break;		
		///< [ptr]: char* with new program name, limited to #kVstMaxProgNameLen  @see AudioEffect::setProgramName
	case effSetProgramName: debug = "effSetProgramName"; break;	
		///< [ptr]: char buffer for current program name, limited to #kVstMaxProgNameLen  @see AudioEffect::getProgramName
	case effGetProgramName: debug = "effGetProgramName"; break;	
		///< [ptr]: char buffer for parameter label, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterLabel
	case effGetParamLabel: debug = "effGetParamLabel"; break;	
		///< [ptr]: char buffer for parameter display, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterDisplay
	//case effGetParamDisplay: debug = "effGetParamDisplay"; break;	
		///< [ptr]: char buffer for parameter name, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterName
	case effGetParamName: debug = "effGetParamName"; break;	
		///< [opt]: new sample rate for audio processing  @see AudioEffect::setSampleRate
	case effSetSampleRate: debug = "effSetSampleRate"; break;	
		///< [value]: new maximum block size for audio processing  @see AudioEffect::setBlockSize
	case effSetBlockSize: debug = "effSetBlockSize"; break;	
		///< [value]: 0 means "turn off", 1 means "turn on"  @see AudioEffect::suspend @see AudioEffect::resume
	case effMainsChanged: debug = "effMainsChanged"; break;	
		///< [ptr]: #ERect** receiving pointer to editor size  @see ERect @see AEffEditor::getRect
	case effEditGetRect: debug = "effEditGetRect"; break;
		///< [ptr]: system dependent Window pointer, e.g. HWND on Windows  @see AEffEditor::open;		
	case effEditOpen: debug = "effEditOpen"; break;		
		///< no arguments @see AEffEditor::close
	case effEditClose: debug = "effEditClose"; break;
		///< no arguments @see AEffEditor::idle
	case effEditIdle: debug = ""; break;		
		///< [ptr]: void** for chunk data address [index]: 0 for bank, 1 for program  @see AudioEffect::getChunk
	case effGetChunk: debug = "effGetChunk"; break;
		///< [ptr]: chunk data [value]: byte size [index]: 0 for bank, 1 for program  @see AudioEffect::setChunk
	case effSetChunk: debug = ""; break;		
		///< [ptr]: #VstEvents*  @see AudioEffectX::processEvents
	case effProcessEvents: debug = "effProcessEvents"; break;		
		///< [index]: parameter index [return value]: 1=true, 0=false  @see AudioEffectX::canParameterBeAutomated
	case effCanBeAutomated: debug = "effCanBeAutomated"; break; 						
		///< [index]: parameter index [ptr]: parameter string [return value]: true for success  @see AudioEffectX::string2parameter	
	case effString2Parameter: debug = "effString2Parameter"; break; 					
		///< [index]: program index [ptr]: buffer for program name, limited to #kVstMaxProgNameLen [return value]: true for success  @see AudioEffectX::getProgramNameIndexed
	case effGetProgramNameIndexed: debug = "effGetProgramNameIndexed"; break; 				
		///< [index]: input index [ptr]: #VstPinProperties* [return value]: 1 if supported  @see AudioEffectX::getInputProperties
	case effGetInputProperties: debug = "effGetInputProperties"; break; 					
		///< [index]: output index [ptr]: #VstPinProperties* [return value]: 1 if supported  @see AudioEffectX::getOutputProperties
	case effGetOutputProperties: debug = "effGetOutputProperties"; break; 				
		///< [return value]: category  @see VstPlugCategory @see AudioEffectX::getPlugCategory
	case effGetPlugCategory: debug = "effGetPlugCategory"; break; 					
		///< [ptr]: #VstAudioFile array [value]: count [index]: start flag  @see AudioEffectX::offlineNotify
	case effOfflineNotify: debug = "effOfflineNotify"; break; 						
		///< [ptr]: #VstOfflineTask array [value]: count  @see AudioEffectX::offlinePrepare
	case effOfflinePrepare: debug = "effOfflinePrepare"; break; 						
		///< [ptr]: #VstOfflineTask array [value]: count  @see AudioEffectX::offlineRun
	case effOfflineRun: debug = "effOfflineRun"; break; 							
		///< [ptr]: #VstVariableIo*  @see AudioEffectX::processVariableIo
	case effProcessVarIo: debug = "effProcessVarIo"; break; 						
		///< [value]: input #VstSpeakerArrangement* [ptr]: output #VstSpeakerArrangement*  @see AudioEffectX::setSpeakerArrangement
	case effSetSpeakerArrangement: debug = "effSetSpeakerArrangement"; break; 				
		///< [value]: 1 = bypass, 0 = no bypass  @see AudioEffectX::setBypass
	case effSetBypass: debug = "effSetBypass"; break; 							
		///< [ptr]: buffer for effect name, limited to #kVstMaxEffectNameLen  @see AudioEffectX::getEffectName
	case effGetEffectName: debug = "effGetEffectName"; break; 						
		///< [ptr]: buffer for effect vendor string, limited to #kVstMaxVendorStrLen  @see AudioEffectX::getVendorString
	case effGetVendorString: debug = "effGetVendorString"; break; 					
		///< [ptr]: buffer for effect vendor string, limited to #kVstMaxProductStrLen  @see AudioEffectX::getProductString
	case effGetProductString: debug = "effGetProductString"; break; 					
		///< [return value]: vendor-specific version  @see AudioEffectX::getVendorVersion
	case effGetVendorVersion: debug = "effGetVendorVersion"; break; 					
		///< no definition, vendor specific handling  @see AudioEffectX::vendorSpecific
	case effVendorSpecific: debug = "effVendorSpecific"; break; 						
		///< [ptr]: "can do" string [return value]: 0: "don't know" -1: "no" 1: "yes"  @see AudioEffectX::canDo#
	case effCanDo: debug = "effCanDo"; break; 								
		///< [return value]: tail size (for example the reverb time of a reverb plug-in); 0 is default (return 1 for 'no tail')
	case effGetTailSize: debug = "effGetTailSize"; break; 						
		///< [index]: parameter index [ptr]: #VstParameterProperties* [return value]: 1 if supported  @see AudioEffectX::getParameterProperties
	case effGetParameterProperties: debug = "effGetParameterProperties"; break; 				
		///< [return value]: VST version  @see AudioEffectX::getVstVersion
	case effGetVstVersion: debug = "effGetVstVersion"; break; 						
		///< \deprecated deprecated in VST 2.4
		// DECLARE_VST_DEPRECATED (effGetVu)
		// DECLARE_VST_DEPRECATED (effEditTop)
		// DECLARE_VST_DEPRECATED (effEditSleep)
		// DECLARE_VST_DEPRECATED (effIdentify)
		// DECLARE_VST_DEPRECATED (effGetNumProgramCategories)			
		// DECLARE_VST_DEPRECATED (effCopyProgram)
		// DECLARE_VST_DEPRECATED (effConnectInput)
		// DECLARE_VST_DEPRECATED (effConnectOutput)
		// DECLARE_VST_DEPRECATED (effEditDraw)
		// DECLARE_VST_DEPRECATED (effEditMouse)
		// DECLARE_VST_DEPRECATED (effEditKey)
		// DECLARE_VST_DEPRECATED (effGetCurrentPosition)
		// DECLARE_VST_DEPRECATED (effGetDestinationBuffer)
		// DECLARE_VST_DEPRECATED (effSetBlockSizeAndSampleRate)
		// DECLARE_VST_DEPRECATED (effGetErrorText)
		// DECLARE_VST_DEPRECATED (effIdle)
		// DECLARE_VST_DEPRECATED (effGetIcon)
		// DECLARE_VST_DEPRECATED (effSetViewPosition)
		// DECLARE_VST_DEPRECATED (effKeysRequired)
#if VST_2_1_EXTENSIONS
	case effEditKeyDown: debug = "effEditKeyDown"; break; 						///< [index]: ASCII character [value]: virtual key [opt]: modifiers [return value]: 1 if key used  @see AEffEditor::onKeyDown
	case effEditKeyUp: debug = "effEditKeyUp"; break; 							///< [index]: ASCII character [value]: virtual key [opt]: modifiers [return value]: 1 if key used  @see AEffEditor::onKeyUp
	case effSetEditKnobMode: debug = "effSetEditKnobMode"; break; 					///< [value]: knob mode 0: circular, 1: circular relativ, 2: linear (CKnobMode in VSTGUI)  @see AEffEditor::setKnobMode
	case effGetMidiProgramName: debug = "effGetMidiProgramName"; break; 					///< [index]: MIDI channel [ptr]: #MidiProgramName* [return value]: number of used programs, 0 if unsupported  @see AudioEffectX::getMidiProgramName
	case effGetCurrentMidiProgram: debug = "effGetCurrentMidiProgram"; break; 				///< [index]: MIDI channel [ptr]: #MidiProgramName* [return value]: index of current program  @see AudioEffectX::getCurrentMidiProgram
	case effGetMidiProgramCategory: debug = "effGetMidiProgramCategory"; break; 				///< [index]: MIDI channel [ptr]: #MidiProgramCategory* [return value]: number of used categories, 0 if unsupported  @see AudioEffectX::getMidiProgramCategory
	case effHasMidiProgramsChanged: debug = "effHasMidiProgramsChanged"; break; 				///< [index]: MIDI channel [return value]: 1 if the #MidiProgramName(s) or #MidiKeyName(s) have changed  @see AudioEffectX::hasMidiProgramsChanged
	case effGetMidiKeyName: debug = "effGetMidiKeyName"; break; 						///< [index]: MIDI channel [ptr]: #MidiKeyName* [return value]: true if supported, false otherwise  @see AudioEffectX::getMidiKeyName
	case effBeginSetProgram: debug = "effBeginSetProgram"; break; 					///< no arguments  @see AudioEffectX::beginSetProgram
	case effEndSetProgram: debug = "effEndSetProgram"; break; 						///< no arguments  @see AudioEffectX::endSetProgram
#endif // VST_2_1_EXTENSIONS
#if VST_2_3_EXTENSIONS
	case effGetSpeakerArrangement: debug = "effGetSpeakerArrangement"; break; 				///< [value]: input #VstSpeakerArrangement* [ptr]: output #VstSpeakerArrangement*  @see AudioEffectX::getSpeakerArrangement
	case effShellGetNextPlugin: debug = "effShellGetNextPlugin"; break; 					///< [ptr]: buffer for plug-in name, limited to #kVstMaxProductStrLen [return value]: next plugin's uniqueID  @see AudioEffectX::getNextShellPlugin
	case effStartProcess: debug = "effStartProcess"; break; 						///< no arguments  @see AudioEffectX::startProcess
	case effStopProcess: debug = "effStopProcess"; break; 						///< no arguments  @see AudioEffectX::stopProcess
	case effSetTotalSampleToProcess: debug = "effSetTotalSampleToProcess"; break; 		    ///< [value]: number of samples to process, offline only!  @see AudioEffectX::setTotalSampleToProcess
	case effSetPanLaw: debug = "effSetPanLaw"; break; 							///< [value]: pan law [opt]: gain  @see VstPanLawType @see AudioEffectX::setPanLaw
	case effBeginLoadBank: debug = "effBeginLoadBank"; break; 						///< [ptr]: #VstPatchChunkInfo* [return value]: -1: bank can't be loaded, 1: bank can be loaded, 0: unsupported  @see AudioEffectX::beginLoadBank
	case effBeginLoadProgram: debug = "effBeginLoadProgram"; break; 					///< [ptr]: #VstPatchChunkInfo* [return value]: -1: prog can't be loaded, 1: prog can be loaded, 0: unsupported  @see AudioEffectX::beginLoadProgram
#endif // VST_2_3_EXTENSIONS
#if VST_2_4_EXTENSIONS
	case effSetProcessPrecision: debug = "effSetProcessPrecision"; break; 				///< [value]: @see VstProcessPrecision  @see AudioEffectX::setProcessPrecision
	case effGetNumMidiInputChannels: debug = "effGetNumMidiInputChannels"; break; 			///< [return value]: number of used MIDI input channels (1-15)  @see AudioEffectX::getNumMidiInputChannels
	case effGetNumMidiOutputChannels: debug = "effGetNumMidiOutputChannels"; break; 			///< [return value]: number of used MIDI output channels (1-15)  @see AudioEffectX::getNumMidiOutputChannels
#endif // VST_2_4_EXTENSIONS
	}
	if (!debug.empty()) {
		printf("%s\n", debug.c_str());
	}
	return AudioEffectX::dispatcher(opcode, index, value, ptr, opt);
}
#endif // DISPATCHER_DEBUG_TRACE
#endif // DEBUG
