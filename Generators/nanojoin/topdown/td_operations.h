#ifndef TD_OPERATIONS_H
#define TD_OPERATIONS_H

unsigned char fill(struct td_patch *patch);
void split(struct td_patch* patch, struct edge *mark);
void unfold(struct td_patch* patch, struct edge *mark);
void unwrap(struct td_patch* patch, struct edge *mark);

void createSpecialFace(struct td_patch *patch, struct edge *from, int nrofedges, int lparameter, int mparameter, int offset);
void connect(struct td_patch* patch, struct edge* from, struct edge* to, int nrofedges, unsigned char filling);
#endif /* TD_OPERATIONS_H */
