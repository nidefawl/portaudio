#ifndef PA_ASIO_H
#define PA_ASIO_H
/*
 * $Id$
 * PortAudio Portable Real-Time Audio Library
 * ASIO specific extensions
 *
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */


/** @file
 @ingroup public_header
 @brief ASIO-specific PortAudio API extension header file.
*/

#include "portaudio.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/** Retrieve legal native buffer sizes for the specified device, in sample frames.

 @param device The global index of the device about which the query is being made.
 @param minBufferSizeFrames A pointer to the location which will receive the minimum buffer size value.
 @param maxBufferSizeFrames A pointer to the location which will receive the maximum buffer size value.
 @param preferredBufferSizeFrames A pointer to the location which will receive the preferred buffer size value.
 @param granularity A pointer to the location which will receive the "granularity". This value determines
 the step size used to compute the legal values between minBufferSizeFrames and maxBufferSizeFrames.
 If granularity is -1 then available buffer size values are powers of two.

 @see ASIOGetBufferSize in the ASIO SDK.

 @note: this function used to be called PaAsio_GetAvailableLatencyValues. There is a
 #define that maps PaAsio_GetAvailableLatencyValues to this function for backwards compatibility.
*/
PaError PaAsio_GetAvailableBufferSizes( PaDeviceIndex device,
        long *minBufferSizeFrames, long *maxBufferSizeFrames, long *preferredBufferSizeFrames, long *granularity );


/** Backwards compatibility alias for PaAsio_GetAvailableBufferSizes

 @see PaAsio_GetAvailableBufferSizes
*/
#define PaAsio_GetAvailableLatencyValues PaAsio_GetAvailableBufferSizes


/** Display the ASIO control panel for the specified device.

  @param device The global index of the device whose control panel is to be displayed.
  @param systemSpecific On Windows, the calling application's main window handle,
  on Macintosh this value should be zero.
*/
PaError PaAsio_ShowControlPanel( PaDeviceIndex device, void* systemSpecific );



/** ASIO message types.

 These mostly correspond with asioMessage calls from the ASIO SDK.
 ASIO's sampleRateDidChange is adapted to use this callback.
 Refer to ASIO SDK documentation for complete information.
*/
typedef enum PaAsioMessageType
{
    /** The driver requests that it be reset (by closing and re-opening the stream).
        Typically dispatched when the user changes driver settings.
        Recommend closing, re-opening and restarting stream, and always returning 1.

        params:
            none.
    */
    paAsioResetRequest      = 1,

    /** Informs the application that a sample-rate change was detected.
        Recommend noting the new sample-rate, but no action is needed.

        params:
            opt[0] -- the new sample-rate.
    */
    paAsioSampleRateChanged = 2,

    /** Informs the application that the driver has a new preferred buffer size.
        Recommend handling like paAsioResetRequest.

        params:
            value -- the new preferred buffer size.
    */
    paAsioBufferSizeChange  = 3,

    /** Informs the application that the driver has gone out of sync, invalidating timestamps.
        Recommend handling like paAsioResetRequest.

        params:
            none.
    */
    paAsioResyncRequest     = 4,

    /** Informs the application that the driver's latencies have changed.
        FIXME: The only way to query the new latencies is to reset the stream.
        Recommend ignoring unless latency reporting is critical.

        params:
            none.
    */
    paAsioLatenciesChanged = 5,

} PaAsioMessageType;

/** ASIO message callback, set in PaAsioStreamInfo.
    Do not call PortAudio or PaAsio functions inside this callback!

 @param value Message-specific integer value.
 Indicates buffer size in paAsioBufferSizeChange.

 @param message Message-specific pointer value.
 Unused as of the ASIO 2.2 SDK.

 @param opt Message-specific double value.
 opt[0] indicates sample rate in paAsioSampleRateChange.

 @param userData The value of a user supplied pointer passed to
 Pa_OpenStream() intended for storing synthesis data etc.

 @return True if the application handled the message, false otherwise.
*/
typedef long PaAsio_MessageCallback( long messageType, long value, void *message, double *opt, void *userData );

/** Retrieve a pointer to a string containing the name of the specified
 input channel. The string is valid until Pa_Terminate is called.

 The string will be no longer than 32 characters including the null terminator.
*/
PaError PaAsio_GetInputChannelName( PaDeviceIndex device, int channelIndex,
        const char** channelName );


/** Retrieve a pointer to a string containing the name of the specified
 input channel. The string is valid until Pa_Terminate is called.

 The string will be no longer than 32 characters including the null terminator.
*/
PaError PaAsio_GetOutputChannelName( PaDeviceIndex device, int channelIndex,
        const char** channelName );


/** Set the sample rate of an open paASIO stream.

 @param stream The stream to operate on.
 @param sampleRate The new sample rate.

 Note that this function may fail if the stream is already running and the
 ASIO driver does not support switching the sample rate of a running stream.

 Returns paIncompatibleStreamHostApi if stream is not a paASIO stream.
*/
PaError PaAsio_SetStreamSampleRate( PaStream* stream, double sampleRate );


#define paAsioUseChannelSelectors      (0x01)
#define paAsioUseMessageCallback       (0x02)

typedef struct PaAsioStreamInfo{
    unsigned long size;             /**< sizeof(PaAsioStreamInfo) */
    PaHostApiTypeId hostApiType;    /**< paASIO */
    unsigned long version;          /**< 2 */

    unsigned long flags;

    /* Support for opening only specific channels of an ASIO device.
        If the paAsioUseChannelSelectors flag is set, channelSelectors is a
        pointer to an array of integers specifying the device channels to use.
        When used, the length of the channelSelectors array must match the
        corresponding channelCount parameter to Pa_OpenStream() otherwise a
        crash may result.
        The values in the selectors array must specify channels within the
        range of supported channels for the device or paInvalidChannelCount will
        result.
    */
    int *channelSelectors;

    /** ASIO message callback.
        Include paAsioUseMessageCallback in flags to enable.
        Unsupported in Blocking I/O mode.
        If a callback is supplied for both input and output, it will be called twice!
    */
    PaAsio_MessageCallback *messageCallback;
}PaAsioStreamInfo;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PA_ASIO_H */
