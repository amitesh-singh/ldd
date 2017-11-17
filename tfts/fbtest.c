#include <linux/fb.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>


 
const char *FB_NAME = "/dev/fb1";
void*   m_FrameBuffer;
struct  fb_fix_screeninfo m_FixInfo;
struct  fb_var_screeninfo m_VarInfo;
int 	m_FBFD;

int main() {

    int iFrameBufferSize;
    /* Open the framebuffer device in read write */
    m_FBFD = open(FB_NAME, O_RDWR);
    if (m_FBFD < 0) {
    	printf("Unable to open %s.\n", FB_NAME);
    	return 1;
    }
    /* Do Ioctl. Retrieve fixed screen info. */
    if (ioctl(m_FBFD, FBIOGET_FSCREENINFO, &m_FixInfo) < 0) {
    	printf("get fixed screen info failed: %s\n",
    		  strerror(errno));
    	close(m_FBFD);
    	return 1;
    }
    /* Do Ioctl. Get the variable screen info. */
    if (ioctl(m_FBFD, FBIOGET_VSCREENINFO, &m_VarInfo) < 0) {
    	printf("Unable to retrieve variable screen info: %s\n",
    		  strerror(errno));
    	close(m_FBFD);
    	return 1;
    }

    /* Calculate the size to mmap */
    iFrameBufferSize = m_FixInfo.line_length * m_VarInfo.yres;
    printf("Line length %d, framebuffer size: %d\n", m_FixInfo.line_length, iFrameBufferSize);
    /* Now mmap the framebuffer. */
    m_FrameBuffer = mmap(NULL, iFrameBufferSize, PROT_READ | PROT_WRITE,
    				 MAP_SHARED, m_FBFD,0);
    if (m_FrameBuffer == NULL) {
    	printf("mmap failed:\n");
    	close(m_FBFD);
    	return 1;
    }

    
    {
        char *p = (char *) m_FrameBuffer;
        int x,y,t=0;
        while(1) {
            for (y=0; y<m_VarInfo.yres; y++) {
                for (x=0; x<m_VarInfo.xres; x++) {
                    p[x + y * m_VarInfo.xres] = 0x00100 * y + x + t;
                }
            }
            t++;
        }
    }   
    

}
