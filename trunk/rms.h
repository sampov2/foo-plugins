#ifndef RMS_H
#define RMS_H

typedef struct _rms rms;

/*
 * rms_new: return an object ecapsulating the RMS state.
 * params: fs, the same rate, in Hz
 *         time, averaging time
 */
rms *rms_new(float fs, float time);

/*
 * rms_set_time: change averaging time.
 * params: r, an RMS object allocated by rms_new
 *         time, averaging time
 */
void rms_set_time(rms *r, float time);

/*
 * rms_run: push one sample into the RMS object, returns the current RMS value.
 * params: r, an RMS object allocated by rms_new
 *         x, the input sample
 */
float rms_run(rms *r, float x);

/* rms_run_buffer: push an array of samples through the RMS object, returning
 * the RMS value after processing the whole buffer.
 * params: r, an RMS object allocated by rms_new
 *         x, and array of input samples
 *         length, length of x in value
 */
float rms_run_buffer(rms *r, float *x, int length);


/*
 * free the space allocated by the rms object.
 */
void rms_free(rms *r);

#endif
