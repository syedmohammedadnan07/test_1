#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#define SAMPLE_RATE 58000
#define FRAMES_PER_BUFFER 512
#define NUM_SECONDS 4.5
#define NUM_CHANNELS 1

/* Data structure to pass to callback */
typedef struct {
    float *recordedSamples;
    int frameIndex;
    int maxFrameIndex;
} UserData;

/* Audio callback function */
static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
    UserData *data = (UserData*)userData;
    const float *rptr = (const float*)inputBuffer;
    float *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc = framesPerBuffer;

    if (data->frameIndex + framesToCalc > data->maxFrameIndex) {
        framesToCalc = data->maxFrameIndex - data->frameIndex;
    }

    if (inputBuffer == NULL) {
        for (int i = 0; i < framesToCalc; i++) {
            *wptr++ = 0;  // Silent audio
        }
    } else {
        for (int i = 0; i < framesToCalc; i++) {
            *wptr++ = *rptr++;  // Copy audio
        }
    }

    data->frameIndex += framesToCalc;
    return paContinue;
}

int main(void) {
    PaStreamParameters inputParameters;
    PaStream *stream;
    PaError err;
    UserData data;
    int numSamples;
    int numBytes;

    numSamples = NUM_SECONDS * SAMPLE_RATE; // Total number of samples to record
    numBytes = numSamples * sizeof(float); // Size in bytes
    data.recordedSamples = (float *)malloc(numBytes); // Allocate array
    if (data.recordedSamples == NULL) {
        printf("Could not allocate record array.\n");
        exit(1);
    }
    for (int i = 0; i < numSamples; i++) {
        data.recordedSamples[i] = 0;
    }

    data.frameIndex = 0;
    data.maxFrameIndex = numSamples;

    err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        exit(1);
    }

    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        exit(1);
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,
              SAMPLE_RATE,
          FRAMES_PER_BUFFER,
          paClipOff,
          recordCallback,
          &data);

if (err != paNoError) {
    printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    exit(1);
}

err = Pa_StartStream(stream);
if (err != paNoError) {
    printf("PortAudio error: start stream: %s\n", Pa_GetErrorText(err));
    exit(1);
}

printf("Recording...\n");
while (Pa_IsStreamActive(stream)) {
    Pa_Sleep(1000);
}

err = Pa_CloseStream(stream);
if (err != paNoError) {
    printf("PortAudio error: close stream: %s\n", Pa_GetErrorText(err));
    exit(1);
}

Pa_Terminate();
printf("Recording complete.\n");

// Save recorded data to a file
FILE *fp = fopen("recorded.raw", "wb");
if (fp == NULL) {
    printf("Unable to open file 'recorded.raw'\n");
    exit(1);
}
fwrite(data.recordedSamples, sizeof(float), numSamples, fp);
fclose(fp);
free(data.recordedSamples);

printf("Data saved to 'recorded.raw'\n");

return 0;
