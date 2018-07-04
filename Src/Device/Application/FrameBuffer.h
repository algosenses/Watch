#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

void InitialiazeFrameBuffer(void);
void WriteBufferHandler(tHostMsg* pMsg);
unsigned char UpdateDisplayHandler(tHostMsg* pMsg);

#endif /* FRAME_BUFFER_H */
