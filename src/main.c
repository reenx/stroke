#include <pebble.h>

Window *my_window;
TextLayer *stroke_count_layer;
TextLayer *lap_count_layer;
TextLayer *stroke_time_layer;
int StrokeStatus = 0;
int StrokeCount = 0;
int LapCount = 0;
CompassHeadingData compasdata;
long int heading = 999;
static char strokecounttext[5];
static char lapcounttext[5];
static char stroketimetext[10];
time_t new_seconds = 0;
uint16_t new_milliseconds = 0;
time_t prev_seconds = 0;
uint16_t prev_milliseconds = 0;

uint16_t time_diff_ms() {
   uint16_t diff_milliseconds;
   time_t diff_seconds = new_seconds - prev_seconds;
   diff_milliseconds = diff_seconds * 1000;
   return diff_milliseconds + new_milliseconds - prev_milliseconds;
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {

   for (int i=0; i < (int)num_samples; i++){
   
      if(!data[i].did_vibrate) {
         // Print it out
//          APP_LOG(APP_LOG_LEVEL_DEBUG, "t: %lu, x: %d, y: %d, z: %d",
//                  (unsigned long)(data[i].timestamp),
//                  data[i].x,
//                  data[i].y,
//                  data[i].z);

         if ((data[i].y < 250) && (data[i].y > -250)) {
            //get compass data
            compass_service_peek(&compasdata);
            if (compasdata.compass_status == CompassStatusCalibrated) {
               heading = TRIGANGLE_TO_DEG(compasdata.magnetic_heading);
            } else {
               heading = 999;
            }
         }

         if(data[i].y > 500) {
            StrokeStatus = 0;
         } 
         if ((data[i].y < -500) && (StrokeStatus == 0)) {
            StrokeStatus = 1;
            StrokeCount++;
            snprintf(strokecounttext, sizeof(strokecounttext),"%d", StrokeCount);
            text_layer_set_text(stroke_count_layer, strokecounttext);
            time_ms(&new_seconds, &new_milliseconds);
            if (prev_milliseconds != 0) {
               uint16_t stroketime = time_diff_ms();
               snprintf(stroketimetext, sizeof(stroketimetext),"%d", stroketime);
               text_layer_set_text(stroke_time_layer, stroketimetext);
               
               if (stroketime > 6000){
                  LapCount++;
                  snprintf(lapcounttext, sizeof(lapcounttext),"%d", LapCount);
                  text_layer_set_text(lap_count_layer, lapcounttext);
               }
                  
            }
            prev_seconds = new_seconds;
            prev_milliseconds = new_milliseconds;
         }
        
                                                             
      } else {
         // Discard with a warning
         APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
      }
   }
   APP_LOG(APP_LOG_LEVEL_INFO, "=== %d = %ld ===", StrokeCount, heading);
}



void handle_init(void) {
   uint32_t num_samples = 5;  // Number of samples per batch/callback
   my_window = window_create();

   stroke_count_layer = text_layer_create(GRect(12, 20, 120, 40));
   stroke_time_layer = text_layer_create(GRect(12, 65, 120, 40));
   lap_count_layer = text_layer_create(GRect(12, 110, 120, 40));
   text_layer_set_text(stroke_count_layer, "000");
   text_layer_set_text(stroke_time_layer, "0000");
   text_layer_set_text(lap_count_layer, "0");
   text_layer_set_text_alignment(stroke_count_layer, GTextAlignmentRight);
   text_layer_set_text_alignment(stroke_time_layer, GTextAlignmentRight);
   text_layer_set_text_alignment(lap_count_layer, GTextAlignmentRight);
   text_layer_set_font(stroke_count_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
   text_layer_set_font(stroke_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
   text_layer_set_font(lap_count_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
   text_layer_set_background_color(stroke_count_layer, GColorGreen);
   text_layer_set_background_color(lap_count_layer, GColorGreen);
   text_layer_set_background_color(stroke_time_layer, GColorYellow);
   Layer *root_layer = window_get_root_layer(my_window);
   layer_add_child(root_layer, text_layer_get_layer(stroke_count_layer));
   layer_add_child(root_layer, text_layer_get_layer(lap_count_layer));
   layer_add_child(root_layer, text_layer_get_layer(stroke_time_layer));

   
   window_stack_push(my_window, true);

   // Subscribe to batched data events
   accel_data_service_subscribe(num_samples, accel_data_handler);

//   // initialize compass and set a filter to 2 degrees
//   compass_service_set_heading_filter(DEG_TO_TRIGANGLE(2));
//   compass_service_subscribe(&compass_heading_handler);   
}

void handle_deinit(void) {
   text_layer_destroy(stroke_count_layer);
//    compass_service_unsubscribe();
   accel_data_service_unsubscribe();
   window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
