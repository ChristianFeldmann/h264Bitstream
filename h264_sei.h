/* 
 * h264bitstream - a library for reading and writing H.264 video
 * Copyright (C) 2005-2007 Auroras Entertainment, LLC
 * Copyright (C) 2008-2011 Avail-TVN
 * 
 * Written by Alex Izvorski <aizvorski@gmail.com> and Alex Giladi <alex.giladi@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdint.h>

#ifndef _H264_SEI_H
#define _H264_SEI_H        1

#include <stdint.h>

#include "bs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int seq_parameter_set_id;
    int nal_parameters_read;
    int nal_initial_cpb_removal_delay[32];
    int nal_initial_cpb_removal_delay_offset[32];
    int vcl_parameters_read;
    int vcl_initial_cpb_removal_delay[32];
    int vcl_initial_cpb_removal_delay_offset[32];
} sei_data_buffering_period;

typedef struct
{
  int cpb_removal_delay;
  int dpb_output_delay;

  int pic_struct;
  int clock_timestamp_flag[3];
  int ct_type[3];
  int nuit_field_based_flag[3];
  int counting_type[3];
  int full_timestamp_flag[3];
  int discontinuity_flag[3];
  int cnt_dropped_flag[3];
  int n_frames[3];

  int NumClockTS;
  int seconds_value[3];
  int minutes_value[3];
  int hours_value[3];
  int seconds_flag[3];
  int minutes_flag[3];
  int hours_flag[3];
  int time_offset[3];

} sei_data_pic_timing;

typedef struct
{
    int payloadType;
    int payloadSize;
    uint8_t* payload;

    void *payload_data;
} sei_t;

sei_t* sei_new();
void sei_free(sei_t* s);

//D.1 SEI payload syntax
#define SEI_TYPE_BUFFERING_PERIOD 0
#define SEI_TYPE_PIC_TIMING       1
#define SEI_TYPE_PAN_SCAN_RECT    2
#define SEI_TYPE_FILLER_PAYLOAD   3
#define SEI_TYPE_USER_DATA_REGISTERED_ITU_T_T35  4
#define SEI_TYPE_USER_DATA_UNREGISTERED  5
#define SEI_TYPE_RECOVERY_POINT   6
#define SEI_TYPE_DEC_REF_PIC_MARKING_REPETITION 7
#define SEI_TYPE_SPARE_PIC        8
#define SEI_TYPE_SCENE_INFO       9
#define SEI_TYPE_SUB_SEQ_INFO    10
#define SEI_TYPE_SUB_SEQ_LAYER_CHARACTERISTICS  11
#define SEI_TYPE_SUB_SEQ_CHARACTERISTICS  12
#define SEI_TYPE_FULL_FRAME_FREEZE  13
#define SEI_TYPE_FULL_FRAME_FREEZE_RELEASE  14
#define SEI_TYPE_FULL_FRAME_SNAPSHOT  15
#define SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_START  16
#define SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_END  17
#define SEI_TYPE_MOTION_CONSTRAINED_SLICE_GROUP_SET  18
#define SEI_TYPE_FILM_GRAIN_CHARACTERISTICS  19
#define SEI_TYPE_DEBLOCKING_FILTER_DISPLAY_PREFERENCE  20
#define SEI_TYPE_STEREO_VIDEO_INFO  21

#ifdef __cplusplus
}
#endif

#endif
