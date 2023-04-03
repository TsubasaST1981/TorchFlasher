#include <rom/tjpgd.h> //TJpgDec (C)ChaN, 2012

char *jpegFiles[5] = {
  "/Picture1.jpg",
  "/Picture2.jpg",
  "/Picture3.jpg",
  "/Picture4.jpg",
  "/Picture5.jpg"
};

#define JD_FORMAT 0
/* Bytes per pixel of image output */
#define N_BPP (3 - JD_FORMAT)

//Bitmap保存構造体
typedef struct {
  const void *fp; /* File pointer for input function */
  BYTE *fbuf;     /* Pointer to the frame buffer for output function */
  UINT wfbuf;     /* Width of the frame buffer [pix] */
} IODEV;

IODEV devid;

//**************************************
UINT in_func (JDEC* jd, BYTE* buff, UINT nbyte)
{
  UINT ret = 0;
  IODEV *dev = (IODEV*)jd->device;   /* Device identifier for the session (5th argument of jd_prepare function) */
  File *f2 = (File *)dev->fp;
  if (buff) {
    return (UINT)f2->read(buff, nbyte);
  } else {
    /* Remove bytes from input stream */
    if (f2->seek(nbyte, SeekCur)) { //seek mode:SeekCur=1(FS.h)
      ret = nbyte;
    } else {
      ret = 0;
    }
  }
  return ret;
}

UINT out_func (JDEC* jd, void* bitmap, JRECT* rect)
{
  IODEV *dev = (IODEV*)jd->device;
  uint8_t *src, *dst;
  uint16_t y, bws;
  unsigned int bwd;

  if (rect->left == 0) {
    Serial.printf("%lu%%\r\n", (rect->top << jd->scale) * 100UL / jd->height);
  }
  src = (uint8_t*)bitmap;
  dst = dev->fbuf + N_BPP * (rect->top * dev->wfbuf + rect->left);
  bws = N_BPP * (rect->right - rect->left + 1);
  bwd = N_BPP * dev->wfbuf;
  for (y = rect->top; y <= rect->bottom; y++) {
    memcpy(dst, src, bws);   /* Copy a line */
    src += bws; dst += bwd;  /* Next line */
  }

  return 1;    /* Continue to decompress */
}

//************************************
bool Jpegtobuff(char *filename) {
  fs::File file = SPIFFS.open(filename, "r");
  if (!file) {
    devid.wfbuf = 0;
    return false;
  }

  void *work;       /* Pointer to the decompressor work area */
  JDEC jdec;        /* Decompression object */
  JRESULT res;      /* Result code of TJpgDec API */
  devid.fp = &file;

  /* Allocate a work area for TJpgDec */
  work = malloc(3500);

  /* Prepare to decompress */
  res = jd_prepare(&jdec, in_func, work, 3100, &devid);
  if (res == JDR_OK) {

    devid.fbuf = (uint8_t*)malloc(N_BPP * jdec.width * jdec.height); //RGB888
    devid.wfbuf = jdec.width;

    res = jd_decomp(&jdec, out_func, 0);   /* Start to decompress */
  }

  free(work);             /* Discard work area */
  file.close();
  
  if (res == JDR_OK) return true;
  devid.wfbuf = 0;
  return false;
}
