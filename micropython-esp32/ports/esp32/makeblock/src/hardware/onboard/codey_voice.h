/**   
 * \par Copyright (C), 2017-2018, MakeBlock
 * \brief     Heard for codey_voice.c.
 * @file      codey_voice.h
 * @author    Leo lu
 * @version    V1.0.0
 * @date      2017/06/28
 *
 * \par Copyright
 * This software is Copyright (C), 2012-2016, MakeBlock. Use is subject to license \n
 * conditions. The main licensing options available are GPL V2 or Commercial: \n
 *
 * \par Open Source Licensing GPL V2
 * This is the appropriate option if you want to share the source code of your \n
 * application with everyone you distribute it to, and you also want to give them \n
 * the right to share who uses it. If you wish to use this software under Open \n
 * Source Licensing, you must contribute all your source code to the open source \n
 * community in accordance with the GPL Version 2 when your application is \n
 * distributed. See http://www.gnu.org/copyleft/gpl.html
 *
 * \par Description
 * This file setup ble to device and start ble device with a makeblock profile.
 *
 * \par Method List:
 *
 *
 * \par History:
 * <pre>
 * `<Author>`         `<Time>`        `<Version>`        `<Descr>`
 * Leo lu             2017/06/28         1.0.0              Initial version
 * </pre>
 *
 */

#ifndef _CODEY_VOICE_H_
#define _CODEY_VOICE_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "codey_utils.h"

/******************************************************************************
 MACRO DEFINITION
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/ 
typedef enum
{
  FILE_MUSIC = 0,
  NOTE_MUSIC,
} codey_music_type_t;
 
typedef struct codey_wav_info
{
  char      wav_head[5];
  uint32_t  file_length;
  char      wav_format_desc[9];
  uint32_t  pcm_wav_format_length;
  uint16_t  encode_format_type;
  uint16_t  number_of_channels;
  uint32_t  sampling_frequency;
  uint32_t  data_transferred_per_second;
  uint16_t  number_of_data_blocks;
  uint16_t  number_of_bits_per_sample;
  uint16_t  additional_optional_information;
  char      fact_optional_desc[5];
  uint32_t  fact_data_length;
  uint8_t   fact_data[8];
  char      data_desc[5];
  uint32_t  wav_data_length;
  uint32_t  wav_head_length;

  codey_music_type_t music_type;
  bool      start_play;
  uint32_t  wav_id;
  uint32_t  play_duration_time_ms;
  uint32_t  play_buffer_stop_tick;

  uint32_t  read_offset;
  bool      is_file_open;
  FIL       fp_music;
  uint32_t  drop_tail_voice;
  uint32_t  drop_step;
  uint32_t  drop_step_cnt;

  uint32_t  note_freq;
  uint32_t  note_duration_ms;
  uint8_t   louder;
  
  struct codey_wav_info *next;
}codey_wav_info_t;

typedef enum
{
  BUZZER = 0,
  GUITAR,
  PIANO,
}codey_voice_note_effect_t;

/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/

/******************************************************************************
 DEFINITION PRIVATE FUNCTIONS
 ******************************************************************************/

/******************************************************************************
 DEFINE PUBLIC FUNCTIONS
 ******************************************************************************/
extern bool codey_voice_init(void);
extern bool codey_void_deinit(void);
extern bool codey_voice_deinit(void);
extern bool codey_voice_parse_wav_file(codey_wav_info_t *wav_info, uint8_t *data_buffer);
extern bool codey_voice_play_file(const char *file_name);
extern bool codey_voice_play_file_to_stop(const char *file_name);
extern void codey_voice_print_wav_info(const codey_wav_info_t *wav_info);
extern void codey_voice_traverse_music_file(void);
extern void codey_voice_remove_nth_file(uint32_t idx);
extern bool codey_voice_play_note(uint32_t freq, uint32_t time_ms, codey_voice_note_effect_t voice_eff, uint8_t louder);
extern bool codey_voice_play_note_to_stop(uint32_t freq, uint32_t time_ms, codey_voice_note_effect_t voice_eff, uint8_t louder);
extern void codey_voice_stop_play(void);
codey_wav_info_t *codey_voice_find_wav_by_id(uint32_t wav_id);

/* add by fftust to set volume */
extern void codey_voice_set_volume_t(float volume);
extern void codey_voice_change_volume_t(float volume);
extern float codey_voice_get_volume_t(void);

/*********************************/
// using example:
/*********************************/

#if 0
// using example
// notice: must wait the flash fat fa inition.

#include "codey_voice.h"

codey_voice_play_file("/music/cat.wav");
codey_voice_play_file("/music/mump.wav");
codey_voice_play_file_to_stop("/music/mump.wav");

// freqent, time spand, voice effect, voice volume
codey_voice_play_note(1000, 1000, BUZZER, 100);
codey_voice_play_note_to_stop(1000, 1000, BUZZER, 100);
#endif

#endif /* _CODEY_VOICE_H_ */


