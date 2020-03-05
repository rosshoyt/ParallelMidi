/*
  ==============================================================================

    NoteHisto.h
    Created: 4 Mar 2020 4:43:45pm
    Author:  Ross Hoyt

  ==============================================================================
*/

#include "GeneralScan.h"

/**
 * @class Histo for the NoteHistoScan reductions -- buckets data into 10 interior ranges
 *              plus two outlier ranges
 */
struct NoteHisto {
    static const int N = 10;
    int bucket[N + 2];
    int hi, lo;
    
    NoteHisto() : hi(100), lo(0) {
        for (int i = 0; i < N + 2; i++)
            bucket[i] = 0;
    }
};

std::ostream &operator<<(std::ostream &out, const NoteHisto &noteHisto) {
    out << "|";
    for (int count: noteHisto.bucket)
        out << count << "|";
    return out;
}

/**
 * Collects a histogram from data.
 */
class NoteHistoScan : public GeneralScan<int, NoteHisto> {
public:
    NoteHistoScan(const std::vector<int> *data) : GeneralScan<int, NoteHisto>(data) {
    }
    
protected:
    virtual NoteHisto init() const {
        NoteHisto h;
        return h;
    }
    
    virtual NoteHisto prepare(const int &datum) const {
        NoteHisto h;
        int bucket_size = (h.hi - h.lo) / h.N;
        if (datum < h.lo)
            h.bucket[0]++;
        else if (datum >= h.hi)
            h.bucket[h.N + 1]++;
        else
            h.bucket[1 + (datum - h.lo) / bucket_size]++;
        return h;
    }
    
    virtual NoteHisto combine(const NoteHisto &left, const NoteHisto &right) const {
        NoteHisto h;
        for (int i = 0; i < h.N + 2; i++)
            h.bucket[i] = left.bucket[i] + right.bucket[i];
        return h;
    }
    
    virtual NoteHisto gen(const NoteHisto &tally) const {
        return tally;
    }
};
