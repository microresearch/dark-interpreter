/*

testbed for dark interpreter
    
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <linux/soundcard.h>
#include <argp.h>

#define FRAMESIZE 2048


int format = AFMT_S16_LE;
int channels=1;
int speed = 44100;
int audio_fd;
int ioctl_frag;


short audio_buffer[FRAMESIZE*2];

int main(void){

/* setup soundcard */

if ((audio_fd =  open("/dev/dsp", O_RDWR,0)) ==-1) {
perror("/dev/dsp");
exit(1);
}

if (ioctl(audio_fd, SNDCTL_DSP_SETDUPLEX, 0) ==-1) {
perror("DUPLEX");
exit(1);
}

ioctl_frag=12;
ioctl_frag +=10*65536;
if (ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &ioctl_frag) ==-1) {
perror("FRAG");
exit(1);
}

if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format) ==-1) {
perror("SETFMT");
exit(1);
}

if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &channels) ==-1) {
perror("CHANNELS");
exit(1);
}

if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed) ==-1) {
perror("SETSPEED");
exit(1);
}

 int len;

while(1){

if ((len = read(audio_fd, &audio_buffer, 1024)) ==-1) {
perror(" audio_read");
exit(1);
}

if ((len = write(audio_fd,&audio_buffer,1024)) ==-1) {
perror(" audio_write");
exit(1);
}

 }
}
