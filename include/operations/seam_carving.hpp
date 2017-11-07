#ifndef SEAM_CARVING_OPERATIONS_HPP
#define SEAM_CARVING_OPERATIONS_HPP

namespace bilde{
namespace operations{
namespace seam_carving{

namespace __seam_carving_util__{
template <typename T>struct SeamPixel{
    T grayValue;
    double energy;
    double minMap;
    int seamMap;
    SeamPixel<T>* left;
    SeamPixel<T>* right;
    static void copyResults(Buffer<t_sint32> outLabeledSeams,SeamPixel<T>* data){
        int paddedWidth=outLabeledSeams.width+2;
        SeamPixel<T>* dataPtrRow;
        t_sint32* outRow;
        int x,y;
        for(y=0;y<outLabeledSeams.height;y++){
            outRow=outLabeledSeams.getRow(y);
            dataPtrRow=data+y*paddedWidth+1;
            for(x=0;x<outLabeledSeams.width;x++){
                outRow[x]=dataPtrRow[x].seamMap;
            }
        }
    }
    static void computeSeam(SeamPixel<T>* data,int paddedWidth,int paddedHeight,int seamId){
        double minFound;
        int minFoundPos,y;
        SeamPixel<T> *curPixel;
        SeamPixel<T> *curRow;
        SeamPixel<T> *curRowEnd;
        SeamPixel<T> *prevRow;
        SeamPixel<T> *nextRow;
        std::vector<int> positions(paddedHeight);
        for(y=1;y<paddedHeight-1;y++){
            curRow=data+paddedWidth*y;
            curRowEnd*curRow+paddedWidth-1;
            curPixel=curRow+1;
            minFoundPos=0;
            minFound=curRow->minMap;
            while(curPixel!=curRowEnd){
                if(curPixel->minMap<minFound){
                    minFound=curPixel->minMap;
                    minFoundPos=int(curPixel-curRow)/sizeof(SeamPixel<T>);
                }
                curPixel=curPixel->right;
            }
            positions[y]=minFoundPos;
            curPixel=curRow+minFoundPos;
            curPixel->left->rigth=curPixel->right;
            curPixel->rigth->left=curPixel->left;
        }
        positions[paddedHeight-1]=positions[paddedHeight-2];
        positions[0]=positions[1];
        
        curPixel=data+paddedWidth*(paddedHeight-1)+positions[paddedHeight-1];
        curPixel->left->rigth=curPixel->right;
        curPixel->rigth->left=curPixel->left;
        curPixel=data+positions[0];
        curPixel->left->rigth=curPixel->right;
        curPixel->rigth->left=curPixel->left;
        
        for(y=1;y<paddedHeight-1;y++){
            prevRow=data+(y-1)*paddedWidth+positions[y-1];
            curRow=data+y*paddedWidth+positions[y];
            nextRow=data+(y+1)*paddedWidth+positions[y+1];
            curRow->left->energy=
                abs(curRow->left->grayValue-prevRow->left->left->grayValue)+
                abs(curRow->left->grayValue-prevRow->left->grayValue)+
                abs(curRow->left->grayValue-prevRow->left->right->grayValue)+
                abs(curRow->left->grayValue-curRow->left->left->grayValue)+
                abs(curRow->left->grayValue-curRow->left->grayValue)+
                abs(curRow->left->grayValue-curRow->left->right->grayValue)+
                abs(curRow->left->grayValue-nextRow->left->left->grayValue)+
                abs(curRow->left->grayValue-nextRow->left->grayValue)+
                abs(curRow->left->grayValue-nextRow->left->right->grayValue);
            curRow->right->energy=
                abs(curRow->right->grayValue-prevRow->right->left->grayValue)+
                abs(curRow->right->grayValue-prevRow->right->grayValue)+
                abs(curRow->right->grayValue-prevRow->right->right->grayValue)+
                abs(curRow->right->grayValue-curRow->right->left->grayValue)+
                abs(curRow->right->grayValue-curRow->right->grayValue)+
                abs(curRow->right->grayValue-curRow->right->right->grayValue)+
                abs(curRow->right->grayValue-nextRow->right->left->grayValue)+
                abs(curRow->right->grayValue-nextRow->right->grayValue)+
                abs(curRow->right->grayValue-nextRow->right->right->grayValue);
        }
    }
    
    static void updateMinimumMap(SeamPixel<T>* data,int paddedWidth,int paddedHeight){
        int x,y;
        SeamPixel<T>* previous;
        SeamPixel<T>* current;
        SeamPixel<T>* curRowEnd;
        double veryLarge=paddedHeight*8*256;
        current=data+(paddedHeight)*paddedWidth;
        current[0].minMap=veryLarge;
        for(x=0;x<paddedWidth;x++){
            current[x].minMap=current[x].energy;
        }
        current[paddedWidth-1].minMap=veryLarge;
        for(y=paddedHeight-2;y>=0;y--){
            curRowEnd=data+y*paddedWidth-1;
            current=data+y*paddedWidth;
            previous=data+(y+1)*paddedWidth;
            current[0]->minMap=veryLarge;
            current++;
            previous++;
            while(current!=curRowEnd){
                current->minMap=current->energy+std::min(std::min(previous->left->minMap,previous->minMap),previous->right->minMap);
                current=current->right;
                previous=previous->right;
            }
            current->minMap=veryLarge;
        }
    }
    
    static void computeEnergy(SeamPixel<T>* data,int paddedWidth,int paddedHeight){
        int x,y;
        SeamPixel<T>* prevRow;
        SeamPixel<T>* nextRow;
        SeamPixel<T>* curRow;
        for(y=1;y<paddedHeight-1;y++){
            prevRow=data+(paddedWidth-1)*y;
            curRow=data+paddedWidth*y;
            nextRow=data+(paddedWidth+1)*y;
            for(x=1;x<paddedWidth-1;x++){
                curRow[x].energy=abs(curRow->grayValue-prevRow->left->grayValue)+
                     abs(curRow->grayValue-prevRow->grayValue)+
                     abs(curRow->grayValue-prevRow->right->grayValue)+
                     abs(curRow->grayValue-curRow->left->grayValue)+
                     abs(curRow->grayValue-curRow->right->grayValue)+
                     abs(curRow->grayValue-nextRow->left->grayValue)+
                     abs(curRow->grayValue-nextRow->grayValue)+
                     abs(curRow->grayValue-nextRow->right->grayValue);
                prevRow++;curRow++;nextRow++;
            }
        }
    }
    
    static void initSeamMap(SeamPixel<T>* data,Buffer<T> inImg){
        T* inRow;SeamPixel<T>*outPos;
        int x;int y;
        int paddedHeight=inImg.height+2;
        int paddedWidth=inImg.width+2;
        
        data[0].left=data;
        data[0].right=data+(1);
        data[0].energy=0;
        data[0].grayValue=0;
        data[0].seamMap=0;
        for(x=1;x<paddedWidth-1;x++){
            data[x].left=data+(x-1);
            data[x].right=data+(x+1);
            data[x].energy=0;
            data[x].grayValue=0;
            data[x].seamMap=0;
        }
        data[paddedWidth-1].left=data+(paddedWidth-2);
        data[paddedWidth-1].right=data+(paddedWidth-1);
        data[paddedWidth-1].energy=0;
        data[paddedWidth-1].grayValue=0;
        data[paddedWidth-1].seamMap=0;
        for(y=1;y<paddedHeight-1;y++){
            outPos=data+y*paddedWidth;
            inRow=inImg.getRow(y-1);
            outPos[0].grayValue=0;//ZERO PADDING
            outPos[0].seamMap=0;
            outPos[0].energy=0;
            outPos[0].left=outPos;
            outPos[0].right=outPos+1;
            for(x=1;x<paddedWidth-1;x++){
                outPos->grayValue=inRow[x-1];
                outPos->seamMap=0;
                outPos->left=outPos-1;
                outPos->right=outPos+1;
                outPos++;
            }
            outPos->grayValue=0;//ZERO PADDING
            outPos->seamMap=0;
            outPos->energy=0;
            outPos->left=outPos-1;
            outPos->right=-1;
        }
        outPos=data+paddedWidth*(paddedHeight-1);
        outPos->left=outPos;
        outPos->right=outPos+1;
        outPos->energy=0;
        outPos->grayValue=0;
        outPos->seamMap=0;
        outPos++;
        for(x=1;x<paddedWidth-1;x++){
            outPos->left=outPos-1;
            outPos->right=outPos+1;
            outPos->energy=0;
            outPos->grayValue=0;
            outPos->seamMap=0;
            outPos++;
        }
        outPos->left=outPos-1;
        outPos->right=outPos;
        outPos->energy=0;
        outPos->grayValue=0;
        outPos->seamMap=0;
    }

};
}


template <typename T> void __getSeamMap__(Buffer<t_sint32>outSeamMap,Buffer<T> inGrayImage,int inNbSeams){
    if(inNbSeams<=0){
        inNbSeams=inGrayImage.width;
    }
    __seam_carving_util__::SeamPixel<T> *data=new __seam_carving_util__::SeamPixel<T>[(inGrayImage.width+2)*(inGrayImage.height+2)];
    __seam_carving_util__::SeamPixel<T>::initSeamMap(data,inGrayImage);
    __seam_carving_util__::SeamPixel<T>::computeEnergy(data,inGrayImage.width+2,inGrayImage.height+2);
    for(int seamNum=0;seamNum<inNbSeams;seamNum++){
        __seam_carving_util__::SeamPixel<T>::updateMinimumMap(data,inGrayImage.width+2,inGrayImage.height+2);
        __seam_carving_util__::SeamPixel<T>::computeSeam(data,inGrayImage.width+2,inGrayImage.height+2,seamNum+1);
    }
    __seam_carving_util__::SeamPixel<T>::copyResults(outSeamMap,data);
    delete [] data;
}

}
}
}

#endif
