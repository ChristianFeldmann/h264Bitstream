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

#include "bs.h"
#include "h264_stream.h"
#include "h264_sei.h"

#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset

sei_t* sei_new()
{
    sei_t* s = (sei_t*)malloc(sizeof(sei_t));
    memset(s, 0, sizeof(sei_t));
    s->payload = NULL;
    s->payload_data = NULL;
    return s;
}

void sei_free(sei_t* s)
{
    if ( s->payload != NULL ) free(s->payload);
    if ( s->payload_data != NULL ) free(s->payload_data);
    free(s);
}

void read_sei_end_bits(h264_stream_t* h, bs_t* b )
{
    // if the message doesn't end at a byte border
    if ( !bs_byte_aligned( b ) )
    {
        if ( !bs_read_u1( b ) ) fprintf(stderr, "WARNING: bit_equal_to_one is 0!!!!\n");
        while ( ! bs_byte_aligned( b ) )
        {
            if ( bs_read_u1( b ) ) fprintf(stderr, "WARNING: bit_equal_to_zero is 1!!!!\n");
        }
    }

    read_rbsp_trailing_bits(h, b);
}

// D.1 SEI payload syntax
void read_sei_payload(h264_stream_t* h, bs_t* b, int payloadType, int payloadSize)
{
    sei_t* s = h->sei;

    if (s->payloadType == 0)
    {
      // Read a buffering_period SEI
      sei_data_buffering_period *buff = (sei_data_buffering_period*)malloc(sizeof(sei_data_buffering_period));
      buff->nal_parameters_read = 0;
      buff->vcl_parameters_read = 0;
      for (int i = 0; i < 32; i++)
      {
        buff->nal_initial_cpb_removal_delay[i] = -1;
        buff->nal_initial_cpb_removal_delay_offset[i] = -1;
        buff->vcl_initial_cpb_removal_delay[i] = -1;
        buff->vcl_initial_cpb_removal_delay_offset[i] = -1;
      }

      buff->seq_parameter_set_id = bs_read_ue(b);
      sps_t* sps = h->sps = h->sps_table[buff->seq_parameter_set_id];

      //NalHrdBpPresentFlag
      if (sps->vui.nal_hrd_parameters_present_flag)
      {
        int numReadBits = sps->hrd.initial_cpb_removal_delay_length_minus1 + 1;
        buff->nal_parameters_read = sps->hrd.cpb_cnt_minus1 + 1;
        for( int SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ ) 
        {
          buff->nal_initial_cpb_removal_delay[SchedSelIdx] = bs_read_u(b, numReadBits);
          buff->nal_initial_cpb_removal_delay_offset[SchedSelIdx] = bs_read_u(b, numReadBits);
        }
      }
      if (sps->vui.vcl_hrd_parameters_present_flag)
      {
        int numReadBits = sps->hrd.initial_cpb_removal_delay_length_minus1 + 1;
        buff->vcl_parameters_read = sps->hrd.cpb_cnt_minus1 + 1;
        for( int SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++ ) 
        {
          buff->vcl_initial_cpb_removal_delay[SchedSelIdx] = bs_read_u(b, numReadBits);
          buff->vcl_initial_cpb_removal_delay_offset[SchedSelIdx] = bs_read_u(b, numReadBits);
        }
      }
      
      s->payload_data = buff;
    }
    else if (s->payloadType == 1)
    {
      // Read a pic_timing SEI
      sei_data_pic_timing *timing = (sei_data_pic_timing*)malloc(sizeof(sei_data_pic_timing));
      timing->cpb_removal_delay = -1;
      timing->dpb_output_delay = -1;
      timing->pic_struct = -1;

      if (h->CpbDpbDelaysPresentFlag != 0)
      {
        int numBytes = h->sps->hrd.cpb_removal_delay_length_minus1 + 1;
        timing->cpb_removal_delay = bs_read_u(b, numBytes);
        numBytes = h->sps->hrd.dpb_output_delay_length_minus1 + 1;
        timing->dpb_output_delay = bs_read_u(b, numBytes);
      }

      if (h->sps->vui.pic_struct_present_flag)
      {
        timing->pic_struct = bs_read_u(b, 4);
        // T.Rec D.2.2. 
        timing->NumClockTS = 3;
        if (timing->pic_struct < 3)
          timing->NumClockTS = 1;
        else if (timing->pic_struct < 5 || timing->pic_struct == 7)
          timing->NumClockTS = 2;

        for( int i = 0; i < timing->NumClockTS ; i++ )
        {
          timing->clock_timestamp_flag[i] = bs_read_u(b, 1);
          if( timing->clock_timestamp_flag[i] != 0 )
          {
            timing->ct_type[i] = bs_read_u(b, 2);
            timing->nuit_field_based_flag[i] = bs_read_u(b, 1);
            timing->counting_type[i] = bs_read_u(b, 5);
            timing->full_timestamp_flag[i] = bs_read_u(b, 1);
            timing->discontinuity_flag[i] = bs_read_u(b, 1);
            timing->cnt_dropped_flag[i] = bs_read_u(b, 1);
            timing->n_frames[i] = bs_read_u(b, 8);

            if( timing->full_timestamp_flag[i] != 0 ) 
            {
              timing->seconds_value[i] = bs_read_u(b, 6);
              timing->minutes_value[i] = bs_read_u(b, 6);
              timing->hours_value[i] = bs_read_u(b, 5);
            }
            else
            {
              timing->seconds_flag[i] = bs_read_u(b, 1);
              if (timing->seconds_flag[i] != 0)
              {
                timing->seconds_value[i] = bs_read_u(b, 6);
                timing->minutes_flag[i] = bs_read_u(b, 1);
                if ( timing->minutes_flag[i] != 0 )
                {
                  timing->minutes_value[i] = bs_read_u(b, 6);
                  timing->hours_flag[i] = bs_read_u(b, 1);
                  if (timing->hours_flag[i])
                  {
                    timing->hours_value[i] = bs_read_u(b, 5);
                  }
                }
              }
            }
            if( h->sps->hrd.time_offset_length > 0 )
            {
              timing->time_offset[i] = bs_read_u(b, h->sps->hrd.time_offset_length);
            }
          }
        }

      }

      s->payload_data = timing;
    }
    else
    {
      // Just save the payload bytes
      s->payload = (uint8_t*)malloc(payloadSize);

      for ( int i = 0; i < payloadSize; i++ )
          s->payload[i] = bs_read_u(b, 8);
    }
        
    read_sei_end_bits(h, b);
}

// D.1 SEI payload syntax
void write_sei_payload(h264_stream_t* h, bs_t* b, int payloadType, int payloadSize)
{
    sei_t* s = h->sei;

    int i;
    for ( i = 0; i < s->payloadSize; i++ )
        bs_write_u(b, 8, s->payload[i]);
}


