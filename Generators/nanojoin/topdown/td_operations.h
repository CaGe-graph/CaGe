#ifndef TD_OPERATIONS_H
#define TD_OPERATIONS_H

unsigned char fill(struct td_patch *patch);
void split(struct td_patch* patch, struct edge *mark);
void unfold(struct td_patch* patch, struct edge *mark);
void unwrap(struct td_patch* patch, struct edge *mark);

#endif /* TD_OPERATIONS_H */
