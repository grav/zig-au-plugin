#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdlib.h>
#include <string.h>

extern void processAudio(const float* in_buffer, float* out_buffer, size_t frames, float volume);

typedef struct {
    AudioComponentPlugInInterface pluginInterface;
    AudioComponentInstance instance;
    float volume;
    AudioStreamBasicDescription streamFormat;
    AURenderCallbackStruct renderCallback;
    AudioUnitConnection connection;
    UInt32 maxFrames;
    AUPreset currentPreset;
} MyPluginState;

static OSStatus AUOpen(void *self, AudioComponentInstance compInstance);
static OSStatus AUClose(void *self);
static OSStatus AUInitialize(void *self);
static OSStatus AUUninitialize(void *self);
static OSStatus AUGetPropertyInfo(void *self, AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32 *outDataSize, Boolean *outWritable);
static OSStatus AUGetProperty(void *self, AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void *outData, UInt32 *outDataSize);
static OSStatus AUSetProperty(void *self, AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void *inData, UInt32 inDataSize);
static OSStatus AURender(void *self, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData);
static OSStatus AUReset(void *self, AudioUnitScope scope, AudioUnitElement elem);

static OSStatus AUGetParameter(void *self, AudioUnitParameterID param, AudioUnitScope scope, AudioUnitElement elem, AudioUnitParameterValue *value);
static OSStatus AUSetParameter(void *self, AudioUnitParameterID param, AudioUnitScope scope, AudioUnitElement elem, AudioUnitParameterValue value, UInt32 bufferOffset);

static AudioUnitPropertyListenerProc myListenerProc = NULL;
static void *myListenerUserData = NULL;

static OSStatus AUAddPropertyListener(void *self, AudioUnitPropertyID prop, AudioUnitPropertyListenerProc proc, void *userData) { 
    myListenerProc = proc;
    myListenerUserData = userData;
    return noErr; 
}
static OSStatus AURemovePropertyListener(void *self, AudioUnitPropertyID prop, AudioUnitPropertyListenerProc proc) { 
    myListenerProc = NULL;
    return noErr; 
}

static OSStatus AURemovePropertyListenerWithUserData(void *self, AudioUnitPropertyID prop, AudioUnitPropertyListenerProc proc, void *userData) { 
    if (myListenerProc == proc && myListenerUserData == userData) {
        myListenerProc = NULL;
        myListenerUserData = NULL;
    }
    return noErr; 
}

static AudioComponentMethod AULookup(SInt16 selector) {
    switch (selector) {
        case kAudioUnitInitializeSelect: return (AudioComponentMethod)AUInitialize;
        case kAudioUnitUninitializeSelect: return (AudioComponentMethod)AUUninitialize;
        case kAudioUnitGetPropertyInfoSelect: return (AudioComponentMethod)AUGetPropertyInfo;
        case kAudioUnitGetPropertySelect: return (AudioComponentMethod)AUGetProperty;
        case kAudioUnitSetPropertySelect: return (AudioComponentMethod)AUSetProperty;
        case kAudioUnitRenderSelect: return (AudioComponentMethod)AURender;
        case kAudioUnitResetSelect: return (AudioComponentMethod)AUReset;
        case kAudioUnitAddPropertyListenerSelect: return (AudioComponentMethod)AUAddPropertyListener;
        case kAudioUnitRemovePropertyListenerSelect: return (AudioComponentMethod)AURemovePropertyListener;
        case 18: return (AudioComponentMethod)AURemovePropertyListenerWithUserData;
        case kAudioUnitGetParameterSelect: return (AudioComponentMethod)AUGetParameter;
        case kAudioUnitSetParameterSelect: return (AudioComponentMethod)AUSetParameter;
        default: return NULL;
    }
}

__attribute__((visibility("default")))
void* MyZigPluginFactory(const AudioComponentDescription *inDesc) {
    MyPluginState *state = (MyPluginState *)malloc(sizeof(MyPluginState));
    if (!state) return NULL;
    memset(state, 0, sizeof(MyPluginState));

    state->pluginInterface.Open = AUOpen;
    state->pluginInterface.Close = AUClose;
    state->pluginInterface.Lookup = AULookup;
    state->pluginInterface.reserved = NULL;

    state->streamFormat.mSampleRate = 44100.0;
    state->streamFormat.mFormatID = kAudioFormatLinearPCM;
    state->streamFormat.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
    state->streamFormat.mBytesPerPacket = 4;
    state->streamFormat.mFramesPerPacket = 1;
    state->streamFormat.mBytesPerFrame = 4;
    state->streamFormat.mChannelsPerFrame = 2;
    state->streamFormat.mBitsPerChannel = 32;
    
    state->maxFrames = 512;
    state->volume = 0.5f;
    state->currentPreset.presetNumber = 0;
    state->currentPreset.presetName = CFSTR("Default");

    return state;
}

static OSStatus AUOpen(void *self, AudioComponentInstance compInstance) {
    ((MyPluginState *)self)->instance = compInstance;
    return noErr;
}

static OSStatus AUClose(void *self) {
    free(self);
    return noErr;
}

static OSStatus AUInitialize(void *self) { return noErr; }
static OSStatus AUUninitialize(void *self) { return noErr; }
static OSStatus AUReset(void *self, AudioUnitScope scope, AudioUnitElement elem) { return noErr; }

static OSStatus AUGetParameter(void *self, AudioUnitParameterID param, AudioUnitScope scope, AudioUnitElement elem, AudioUnitParameterValue *value) {
    MyPluginState *state = (MyPluginState *)self;
    if (param == 0) {
        *value = state->volume;
        return noErr;
    }
    return kAudioUnitErr_InvalidParameter;
}

static OSStatus AUSetParameter(void *self, AudioUnitParameterID param, AudioUnitScope scope, AudioUnitElement elem, AudioUnitParameterValue value, UInt32 bufferOffset) {
    MyPluginState *state = (MyPluginState *)self;
    if (param == 0) {
        state->volume = value;
        return noErr;
    }
    return kAudioUnitErr_InvalidParameter;
}

static OSStatus AUGetPropertyInfo(void *self, AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32 *outDataSize, Boolean *outWritable) {
    switch (prop) {
        case kAudioUnitProperty_StreamFormat:
            if (outDataSize) *outDataSize = sizeof(AudioStreamBasicDescription);
            if (outWritable) *outWritable = true;
            return noErr;
        case kAudioUnitProperty_SupportedNumChannels:
            if (outDataSize) *outDataSize = sizeof(AUChannelInfo);
            if (outWritable) *outWritable = false;
            return noErr;
        case kAudioUnitProperty_ElementCount:
            if (outDataSize) *outDataSize = sizeof(UInt32);
            if (outWritable) *outWritable = false;
            return noErr;
        case kAudioUnitProperty_MaximumFramesPerSlice:
            if (outDataSize) *outDataSize = sizeof(UInt32);
            if (outWritable) *outWritable = true;
            return noErr;
        case kAudioUnitProperty_PresentPreset:
            if (outDataSize) *outDataSize = sizeof(AUPreset);
            if (outWritable) *outWritable = true;
            return noErr;
        case kAudioUnitProperty_ClassInfo:
            if (outDataSize) *outDataSize = sizeof(CFPropertyListRef);
            if (outWritable) *outWritable = true;
            return noErr;
        case kAudioUnitProperty_SampleRate:
            if (outDataSize) *outDataSize = sizeof(Float64);
            if (outWritable) *outWritable = true;
            return noErr;
        case kAudioUnitProperty_ParameterList:
            if (outDataSize) *outDataSize = sizeof(AudioUnitParameterID) * 1;
            if (outWritable) *outWritable = false;
            return noErr;
        case kAudioUnitProperty_ParameterInfo:
            if (elem == 0) {
                if (outDataSize) *outDataSize = sizeof(AudioUnitParameterInfo);
                if (outWritable) *outWritable = false;
                return noErr;
            }
            return kAudioUnitErr_InvalidProperty;
    }
    return kAudioUnitErr_InvalidProperty;
}

static OSStatus AUGetProperty(void *self, AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void *outData, UInt32 *outDataSize) {
    MyPluginState *state = (MyPluginState *)self;
    switch (prop) {
        case kAudioUnitProperty_StreamFormat:
            *(AudioStreamBasicDescription *)outData = state->streamFormat;
            return noErr;
        case kAudioUnitProperty_SupportedNumChannels: {
            AUChannelInfo *info = (AUChannelInfo *)outData;
            info->inChannels = 2;
            info->outChannels = 2;
            return noErr;
        }
        case kAudioUnitProperty_ElementCount:
            *(UInt32 *)outData = 1;
            return noErr;
        case kAudioUnitProperty_MaximumFramesPerSlice:
            *(UInt32 *)outData = state->maxFrames;
            return noErr;
        case kAudioUnitProperty_PresentPreset:
            *(AUPreset *)outData = state->currentPreset;
            return noErr;
        case kAudioUnitProperty_ClassInfo: {
            CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            SInt32 type = 'aufx';
            SInt32 subtype = 'volu';
            SInt32 manufacturer = 'Zigg';
            SInt32 version = 65536;
            
            CFNumberRef typeRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &type);
            CFNumberRef subRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &subtype);
            CFNumberRef manRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &manufacturer);
            CFNumberRef verRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &version);
            
            CFDictionarySetValue(dict, CFSTR("type"), typeRef);
            CFDictionarySetValue(dict, CFSTR("subtype"), subRef);
            CFDictionarySetValue(dict, CFSTR("manufacturer"), manRef);
            CFDictionarySetValue(dict, CFSTR("version"), verRef);
            CFDictionarySetValue(dict, CFSTR("name"), CFSTR("Demo: ZigPlugin"));
            
            CFRelease(typeRef);
            CFRelease(subRef);
            CFRelease(manRef);
            CFRelease(verRef);
            
            *(CFPropertyListRef *)outData = dict;
            return noErr;
        }
        case kAudioUnitProperty_SampleRate:
            *(Float64 *)outData = state->streamFormat.mSampleRate;
            return noErr;
        case kAudioUnitProperty_ParameterList:
            if (outDataSize && *outDataSize >= sizeof(AudioUnitParameterID)) {
                ((AudioUnitParameterID*)outData)[0] = 0;
            }
            return noErr;
        case kAudioUnitProperty_ParameterInfo:
            if (elem == 0) {
                AudioUnitParameterInfo *info = (AudioUnitParameterInfo *)outData;
                memset(info, 0, sizeof(AudioUnitParameterInfo));
                strcpy(info->name, "Volume");
                info->cfNameString = CFSTR("Volume");
                info->unit = kAudioUnitParameterUnit_LinearGain;
                info->minValue = 0.0f;
                info->maxValue = 1.0f;
                info->defaultValue = 0.5f;
                info->flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable | kAudioUnitParameterFlag_HasCFNameString;
                return noErr;
            }
            return kAudioUnitErr_InvalidProperty;
    }
    return kAudioUnitErr_InvalidProperty;
}

static OSStatus AUSetProperty(void *self, AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void *inData, UInt32 inDataSize) {
    MyPluginState *state = (MyPluginState *)self;
    OSStatus err = noErr;
    switch (prop) {
        case kAudioUnitProperty_StreamFormat: {
            const AudioStreamBasicDescription *desc = (const AudioStreamBasicDescription *)inData;
            if (desc->mChannelsPerFrame != 2) return kAudioUnitErr_FormatNotSupported;
            state->streamFormat = *desc;
            break;
        }
        case kAudioUnitProperty_SetRenderCallback:
            state->renderCallback = *(const AURenderCallbackStruct *)inData;
            break;
        case kAudioUnitProperty_MakeConnection:
            state->connection = *(const AudioUnitConnection *)inData;
            break;
        case kAudioUnitProperty_MaximumFramesPerSlice:
            state->maxFrames = *(const UInt32 *)inData;
            break;
        case kAudioUnitProperty_PresentPreset:
            state->currentPreset = *(const AUPreset *)inData;
            break;
        case kAudioUnitProperty_ClassInfo:
            break;
        case kAudioUnitProperty_SampleRate:
            state->streamFormat.mSampleRate = *(const Float64 *)inData;
            break;
        default:
            return kAudioUnitErr_InvalidProperty;
    }
    if (myListenerProc) {
        myListenerProc(myListenerUserData, self, prop, scope, elem);
    }
    return err;
}

static float dummyBuffer[8][100000];

static OSStatus AURender(void *self, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) {
    MyPluginState *state = (MyPluginState *)self;
    if (inNumberFrames > 100000) return kAudioUnitErr_TooManyFramesToProcess;

    for (UInt32 i = 0; i < ioData->mNumberBuffers; i++) {
        if (!ioData->mBuffers[i].mData && i < 8) {
            ioData->mBuffers[i].mData = dummyBuffer[i];
        }
    }

    if (state->connection.sourceAudioUnit) {
        OSStatus err = AudioUnitRender(state->connection.sourceAudioUnit, ioActionFlags, inTimeStamp, state->connection.sourceOutputNumber, inNumberFrames, ioData);
        if (err != noErr) return err;
    } else if (state->renderCallback.inputProc) {
        // pull from bus 0
        OSStatus err = state->renderCallback.inputProc(state->renderCallback.inputProcRefCon, ioActionFlags, inTimeStamp, 0, inNumberFrames, ioData);
        if (err != noErr) return err;
    }
    
    for (UInt32 i = 0; i < ioData->mNumberBuffers; i++) {
        float *buffer = (float *)ioData->mBuffers[i].mData;
        if (buffer) {
            processAudio(buffer, buffer, inNumberFrames, state->volume);
        }
    }
    return noErr;
}
