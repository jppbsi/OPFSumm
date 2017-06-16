 #ifndef _SUMMTOOLS_H_
#define _SUMMTOOLS_H_

#include "subgraph.h"

/**
 >>>> COMPLEX TYPES
 */
typedef struct _frame {
    SNode *opfnode;
    struct _frame *prev;
    struct _frame *next;
} Frame;

typedef struct _dissimilarity {
    Frame *frame01;
    Frame *frame02;
    float distance;
    float improvdist;
    struct _dissimilarity *prev;
    struct _dissimilarity *next;
} DissValue;

//--------------------------------------------------------------------------------------

/**
 >>>> METHODS
 */
Frame *InitializeFrame(SNode *opfnode, int nfeats);                                     // Constructor of Frame struct
DissValue *InitializeDissValue(Frame *frame01, Frame *frame02, float dist);             // Constructor of DissValue struct
void DestroyFrame(Frame **frm);                                                         // Destructor of Frame struct
void DestroyDissValue(DissValue **dv);                                                  // Destructor of DissValue struct

Frame *BuildFramesList(Subgraph *g);
DissValue *CalculateDissimilarity(Frame *head, int nfeats);

Frame *AddFrame(Frame *head, Frame *newframe);                                            // Adds a frame  as the last one in the list
void RemoveFrame(Frame *head);                                                          // Removes the last frame of the list

DissValue *AddDissValue(DissValue *head, DissValue *newdv);
void RemoveDissValue(DissValue *head);

#endif // _SUMMTOOLS_H_
