#include <stdlib.h>
#include <unistd.h>
#include <mpd-ng/plugins/post_input.h>

static char name[]   = "rate-convert";
static int  revision = 1;
static int  initial_priority = 100;

FrameGen_I*  RateConvert_getFrameGen(PluginData_T);
PluginData_T RateConvert_init(FrameGen_T*);

void
MpdNG_PostInputPluginInfo(PostInputPluginInfo_T *info)
{
    info->name        = name;
    info->priority    = initial_priority;
    info->revision    = revision;
    info->getFrameGen = RateConvert_getFrameGen;
    info->init        = RateConvert_init;
}

FrameGen_I*
RateConvert_getFrameGen(PluginData_T data)
{
    return NULL;
}

PluginData_T
RateConvert_init(FrameGen_T* input)
{
    return NULL;
}

//
////
//// This function cancel micro fade end for current, cancel microfade start for
//// the next, AND conserve the sample rate structure for the next track.
//// XXX: both files can now NOT be played in the same time. In case of manual
//// change of track, delete next and reload it.
//void
//mm_track_continue_with(
//    struct mm_track_t *current,
//    struct mm_track_t *next)
//{
//    // if sample rates are not the same, it cannot be true
//    if (current->snd_info.samplerate != next->snd_info.samplerate)
//        return;
//
//    // if num channels rates are not the same, it cannot be true
//    if (current->channels != next->channels)
//        return;
//
//    // if end effect has started it is too late
//    if (current->end_effect_started)
//        return;
//
//    // free next ressources
//    free(next->snd_buffer);
//    free(next->out_buffer);
//    free(next->src_data);
//    src_delete(next->src_state);
//
//    // move current to next
//    next->snd_buffer = current->snd_buffer;
//    next->out_buffer = current->out_buffer;
//    next->src_data   = current->src_data;
//    next->src_state  = current->src_state;
//
//    /* Tell current to:
//     * - not free his now shared ressources:
//     * current->snd_buffer
//     * current->out_buffer
//     * current->src_data
//     * current->src_state
//     * - not set the "end" variable on the src_structure
//     * - not run the fast fadout end of file
//     *
//     * Tell next to not start with microfade
//     */
//    current->next_natural = 1;
//    next->disable_start_effect = 1;
//}
//void
//mm_track_fast_end(struct mm_track_t *t)
//{
//    mm_effect_change_span(&t->end_effect, MICRO_FADE_MS);
//}
//
//void
//mm_track_cancel_continue_with(struct mm_track_t *current, struct mm_track_t *next)
//{
//    if (!current)
//        return;
//
//    if (!current->next_natural)
//        return;
//
//    current->next_natural = 0;
//
//    next->snd_buffer = NULL;
//    next->out_buffer = NULL;
//    next->src_data   = NULL;
//    next->src_state  = NULL;
//
//    if (current->position >= current->end_effect_at) {
//        // if we are allready in END effect span, start it now and reduce
//        // effect time to fit the remaining frames
//        int iframe_total =
//            current->end_effect_at + current->end_effect.iframe_span;
//        int iframe_remaining = iframe_total - current->position;
//        current->end_effect_at = current->position;
//        current->end_effect.iframe_span = iframe_remaining;
//        // now the effect should start at the next read
//    }
//}
//
//void
//mm_track_end_with_effect(struct mm_track_t *t, struct mm_effect_t effect)
//{
//    t->end_with_effect = effect;
//}
//
//void
//mm_track_end_now_with_effect(struct mm_track_t *t, struct mm_effect_t effect)
//{
//    if (t->end_effect_started)
//        return;
//
//    int remaining = t->total_iframes - t->position;
//    if (remaining < effect.iframe_span)
//        effect.iframe_span = remaining;
//
//    t->end_with_effect    = effect;
//    t->end_effect_started = 1;
//}
//
//
//struct mm_track_t*
//mm_track_prepare(char *filename, int out_buff_size)
//{
//    struct mm_track_t *track = malloc(sizeof(struct mm_track_t));
//    track->sndfile    = NULL;
//    track->src_state  = NULL;
//    track->src_data   = NULL;
//    track->snd_buffer = NULL;
//    track->out_buffer = NULL;
//    track->prev = track->next = NULL;
//    track->position   = 0;
//    track->end_of_file = 0;
//    track->end_effect_started = 0;
//    track->buffers_size = out_buff_size;
//    track->channels = NUM_CHANNELS;
//    track->buffer_max_out_frames = out_buff_size / NUM_CHANNELS;
//    track->next_natural = 0; // default free everything on clear
//    track->disable_start_effect = 0; // set by mm_track_continue_with
//
//    //
//    // first try to open the sound file
//    //
//
//    //
//    // lsndfile doc: "When opening a file for read, the format field should be
//    // set to zero before calling sf_open()."
//    //
//    track->snd_info.format = 0;
//    track->sndfile = sf_open(filename, SFM_READ, &track->snd_info);
//    if (track->sndfile == NULL)
//    {
//        printf("File open error: %s %s\n",
//            filename, sf_strerror(track->sndfile));
//        mm_track_clear(track);
//        return NULL;
//    }
//
//    //
//    // TODO create a rewind buffer
//    //
//
//    //
//    //
//    //
//    int err;
//    track->src_state = src_new(SRC_SINC_BEST_QUALITY, NUM_CHANNELS, &err);
//    if (track->src_state == NULL)
//    {
//        fprintf(stderr, "src_new failed %s\n", src_strerror(err));
//        mm_track_clear(track);
//        return NULL;
//    }
//
//    //
//    // Allocate buffers so snd_buffer will allways be cleared when sample rate
//    // conversion push data to out_buffer.
//    //
//    track->snd_buffer = malloc(out_buff_size * sizeof(float));
//    track->out_buffer = malloc(out_buff_size * sizeof(float));
//
//    //
//    // Prepare begin/end of file fast fade
//    //
//    sf_seek(track->sndfile, 0, SEEK_END);
//    track->total_iframes = sf_seek(track->sndfile, 0, SEEK_CUR);
//    sf_seek(track->sndfile, 0, SEEK_SET);
//
//    int effect_iframe_span = track->snd_info.samplerate / 1000 * MICRO_FADE_MS;
//
//    mm_effect_configure(
//        &track->start_effect,
//        MM_FADE_TYPE_LINEAR,
//        MM_FADE_ORIENT_IN,
//        NUM_CHANNELS,
//        effect_iframe_span);
//
//    track->end_effect_at = track->total_iframes - effect_iframe_span;
//    mm_effect_configure(
//        &track->end_effect,
//        MM_FADE_TYPE_LINEAR,
//        MM_FADE_ORIENT_OUT,
//        NUM_CHANNELS,
//        effect_iframe_span);
//
//    //
//    // Configure sample rate for our output rate
//    //
//    track->src_data = malloc(sizeof(SRC_DATA));
//    track->src_data->data_in = track->snd_buffer;
//    track->src_data->data_out = track->out_buffer;
//    track->src_data->end_of_input = 0;
//    track->src_data->input_frames = 0;
//    track->src_data->src_ratio = (float) OUTPUT_RATE / track->snd_info.samplerate;
//    track->src_data->output_frames = track->buffer_max_out_frames;
//
//    return track;
//}
//
//int
//mm_track_read(struct mm_track_t *t, int fmax)
//{
//
//    t->src_data->output_frames = fmax;
//    t->src_data->data_out      = t->out_buffer;
//    int ngenerated = 0;
//    while (ngenerated != fmax)
//    {
//        int niframe;
//        //
//        // if converter has consumed all frames, fill it, apply start end
//        // effect...
//        //
//        if (t->src_data->input_frames == 0 && !t->end_effect.done)
//        {
//            //
//            // ...fill the snd_buffer with fresh data...
//            //
//            niframe = sf_readf_float(
//                    t->sndfile,
//                    t->snd_buffer,
//                    t->buffers_size / t->channels);
//
//            //
//            // ...apply start effect if relevent
//            //
//            if (!t->start_effect.done && !t->disable_start_effect) {
//                mm_effect_apply_to_buffer(
//                        &t->start_effect,
//                        t->snd_buffer, niframe);
//            }
//
//            //
//            // ... apply end effect if not natural
//            //
//            if (t->next_natural != 1)
//            {
//                int start_at = 0;
//                //
//                // end not strared...
//                //
//                if (!t->end_effect_started) {
//                    //
//                    // ... have to decide where to start end effect on this
//                    // filled buffer
//                    //
//                    if ((t->position + niframe) >= t->end_effect_at) {
//                        start_at = t->end_effect_at - t->position;
//                        //
//                        // started so the next if will be effective
//                        //
//                        t->end_effect_started = 1;
//                    }
//                }
//
//
//                //
//                // End effect started so do it
//                //
//                if (t->end_effect_started) {
//                    int nproc = mm_effect_apply_to_buffer(
//                            &t->end_effect,
//                            &t->snd_buffer[start_at * t->channels],
//                            niframe - start_at);
//                    //
//                    // If the effect is done, there are unused frames in the buffer
//                    // so: this is the last read so position will not be used anymore
//                    // , set niframe to the value we have
//                    if (t->end_effect.done) {
//                        niframe = (start_at * t->channels) + nproc;
//                    }
//                }
//            }
//
//            t->position += niframe;
//
//            t->src_data->input_frames = niframe;
//            t->src_data->data_in = t->snd_buffer;
//
//            //
//            // End of file
//            //
//            if (t->src_data->input_frames < t->buffers_size / t->channels)
//            {
//                if (!t->next_natural)
//                    t->src_data->end_of_input = SF_TRUE;
//                t->end_of_file = 1;
//            }
//
//        }
//
//        //
//        // Convert with libsamplerate
//        //
//        int err = src_process(t->src_state, t->src_data);
//        if (err)
//        {
//            fprintf(stderr, "src_process error %s\n", src_strerror(err));
//            return 0;
//        }
//
//        if ((t->end_of_file || t->end_effect.done) &&
//                t->src_data->output_frames_gen == 0)
//        {
//            //
//            // Nothing was generated because it IS the very end for both file
//            // buffer and converted sample rate buffer.
//            //
//            break;
//        }
//
//        // Update this (libsamplerate could have done this with a different API)
//        t->src_data->data_in +=
//            t->src_data->input_frames_used * t->channels;
//        t->src_data->input_frames -= t->src_data->input_frames_used;
//
//
//        ngenerated += t->src_data->output_frames_gen;
//        if (ngenerated < fmax && !t->end_effect.done)
//        {
//            t->src_data->output_frames = fmax - ngenerated;
//            t->src_data->data_out = &t->out_buffer[ngenerated * t->channels];
//        } else {
//            break;
//        }
//
//    }
//
//    return ngenerated;
//}
//
//void
//mm_track_clear(struct mm_track_t *t)
//{
//    if (!t->next_natural) {
//        if (t->snd_buffer)
//            free(t->snd_buffer);
//        if (t->out_buffer)
//            free(t->out_buffer);
//        if (t->src_data)
//            free(t->src_data);
//        if (t->src_state)
//            src_delete(t->src_state);
//    }
//    sf_close(t->sndfile);
//    free(t);
//}
//
