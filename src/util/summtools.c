#include "OPF.h"
#include "summtools.h"

/**
    CONSTRUCTOR AND DESTRUCTOR
*/
Frame *InitializeFrame(SNode *opfnode, int nfeats) {
    Frame *frm = (Frame *)calloc(1, sizeof(Frame));

    frm->opfnode = (SNode *)calloc(1,sizeof(SNode));;
    frm->prev = NULL;
    frm->next = NULL;
    CopySNode(frm->opfnode, opfnode, nfeats);

    return frm;
}

DissValue *InitializeDissValue(Frame *frame01, Frame *frame02, float dist) {
    DissValue *dv = (DissValue *)calloc(1, sizeof(DissValue));

    dv->frame01 = frame01;
    dv->frame02 = frame02;
    dv->distance = dist;
    dv->improvdist = 0.0f;
    dv->prev = NULL;
    dv->next = NULL;

    return dv;
}

void DestroyFrame(Frame **frm) {
    if ((*frm) != NULL) {
        (*frm)->prev = NULL;
        (*frm)->next = NULL;
        free((*frm)->opfnode);
        free((*frm));
        (*frm) = NULL;
    }
}

void DestroyDissValue(DissValue **dv) {
    if ((*dv) != NULL) {
        free((*dv)->frame01);
        free((*dv)->frame02);
        (*dv)->prev = NULL;
        (*dv)->next = NULL;
        free((*dv));
        (*dv) = NULL;
    }
}

//--------------------------------------------------------------------------------------

/**
    POPULATE THE LISTS OF FRAMES AND DISSIMILARITY VALUES
*/

Frame *BuildFramesList(Subgraph *g) {
    int i;
    Frame *begin = NULL, *temp = NULL, *newframe;

    if (g == NULL) return NULL;

    for (i = 0; i < g->nnodes; i++) {
        if (begin == NULL) {
            begin = InitializeFrame(&g->node[i], g->nfeats);
            temp = begin;
        } else {
            newframe = InitializeFrame(&g->node[i], g->nfeats);
            temp = AddFrame(temp, newframe);
        }
    }

    return begin;
}

DissValue *CalculateDissimilarity(Frame *head, int nfeats) {
    Frame *frm = head;
    DissValue *begin = NULL, *temp = NULL, *newdv = NULL;
    int i, nframes = 0;
    float dist, minvalue = FLT_MAX, maxvalue = FLT_MIN;

    while (frm != NULL) {                                                               // Finding the total number of frames
        nframes++;
        //printf("\nID: %d", frm->opfnode->position);
        frm = frm->next;
    }

    for (i = 0, dist = 0.0f, frm = head; i < nframes - 1; i++, frm = frm->next) {       // Finding min and max distance values
        dist = opf_EuclDistLog(frm->opfnode->feat, frm->next->opfnode->feat, nfeats);
        if (dist > maxvalue) maxvalue = dist;
        if (dist < minvalue) minvalue = dist;
    }

    for (i = 0, dist = 0.0f, frm = head; i < nframes - 1; i++, frm = frm->next) {      // Populating the dissimilarity list
        dist = opf_EuclDistLog(frm->opfnode->feat, frm->next->opfnode->feat, nfeats);
        dist = (dist - minvalue) / (maxvalue - minvalue);

        if (begin == NULL) {
            begin = InitializeDissValue(frm, frm->next, dist);
            temp = begin;
        } else {
            newdv = InitializeDissValue(frm, frm->next, dist);
            temp = AddDissValue(temp, newdv);
        }

        temp->distance = dist;
    }

    return begin;
}

//--------------------------------------------------------------------------------------

/**
    DOUBLE-LINKED LIST OPERATIONS
*/

Frame *AddFrame(Frame *head, Frame *newframe) {
    newframe->prev = head;
    head->next = newframe;
    head = head->next;

    return head;
}

void RemoveFrame(Frame *head) {
    Frame *temp = head;
    head = head->prev;
    head->next = NULL;
    temp->prev = NULL;
    DestroyFrame(&temp);
}

DissValue *AddDissValue(DissValue *head, DissValue *newdv) {
    newdv->prev = head;
    head->next = newdv;
    head = head->next;

    return head;
}

void RemoveDissValue(DissValue *head) {
    DissValue *temp = head;
    head = head->prev;
    head->next = NULL;
    temp->prev = NULL;
    DestroyDissValue(&temp);
}

//--------------------------------------------------------------------------------------
