#ifndef COMPONENT_SET_HPP
#define COMPONENT_SET_HPP
namespace bilde{

struct LabeledComponent{
    int label;
    int nbPixels;
    int xSum;
    int ySum;
    int top;
    int bottom;
    int left;
    int right;
    int getWidth(){return 1+right-left;}
    int getHeight(){return 1+bottom-top;}

    LabeledComponent(const LabeledComponent& l){
        label=l.label;
        xSum=l.xSum;
        ySum=l.ySum;
        left=l.left;right=l.right;
        top=l.top;bottom=l.bottom;
        nbPixels=l.nbPixels;
    }

    LabeledComponent(){
        label=0;
        xSum=0;
        ySum=0;
        left=0;right=0;
        top=0;bottom=0;
        nbPixels=0;
    }
    LabeledComponent(int lab,int x,int y):label(lab){
        xSum=x;
        ySum=y;
        left=x;right=x;
        top=y;bottom=y;
        nbPixels=1;
    }
    void initialize(int lab,int x,int y){
        label=lab;
        xSum=x;
        ySum=y;
        left=x;right=x;
        top=y;bottom=y;
        nbPixels=1;
    }
    void update(int x,int y){
        xSum+=x;
        ySum+=y;
        top=y*(top<y)+top*(top>=y);
        bottom=y*(bottom>y)+bottom*(bottom<=y);
        left=x*(left<x)+left*(left>=x);
        right=x*(right>x)+right*(right<=x);
        nbPixels++;
    }
};

struct LabeledComponentSet: public Buffer<t_label>{
    std::map<t_label,LabeledComponent> allComponents;
    void __updateComponents__(){
        this->allComponents.clear();
        int x,y;
        t_label* row;
        allComponents[0]=LabeledComponent(0,0,0);
        LabeledComponent& zeroComponent=allComponents[0];
        zeroComponent.right=this->width-1;
        zeroComponent.bottom=this->height-1;
        for(y=0;y<this->width;y++){
            row=this->getRow(y);
            for(x=0;x<this->width;x++){
                if(row[x]!=0){
                    if(allComponents.count(row[x])>0){
                        allComponents[row[x]].update(x,y);
                    }else{
                        allComponents[row[x]]=LabeledComponent(row[x],x,y);
                    }
                }else{
                    zeroComponent.nbPixels++;
                }
            }
        }
    }
    LabeledComponentSet(const Buffer<t_sint32> &labeledImage):Buffer<t_label>(labeledImage.width,labeledImage.height){
        this->copyFrom(labeledImage);
        this->__updateComponents__();
    }
    LabeledComponentSet(int w,int h):Buffer<t_sint32>(w,h){
        for(int row=0;row<h;row++){
            std::memset(this->getRow(row),0,this->width*sizeof(t_label));
        }
        this->allComponents.clear();
        this->allComponents[0]=LabeledComponent(0,0,0);
        this->allComponents[0].right=this->width-1;
        this->allComponents[0].bottom=this->height-1;
        this->allComponents[0].nbPixels=this->width*this->height;
    }
    void eraseComponent(int lab){
        LabeledComponent& c=allComponents[lab];
        int y,x,width;
        t_label* row;
        width=c.getWidth();
        for(y=c.top;y<=c.bottom;y++){
            row=this->getRow(y)+c.left;
            for(x=0;x<width;x++){
                row[x]=row[x]*(row[x]!=lab);
            }
        }
        allComponents[0].nbPixels+=c.nbPixels;
        this->allComponents.erase(lab);
    }
    void moveComponent(int lab,Buffer<t_label> out ){
        LabeledComponent& c=allComponents[lab];
        int y,x,width;
        t_label* row;
        t_label* outRow;
        width=c.getWidth();
        for(y=c.top;y<=c.bottom;y++){
            row=this->getRow(y)+c.left;
            outRow=this->getRow(y)+c.left;
            for(x=0;x<width;x++){
                outRow[x]=row[x]*(row[x]==lab)+outRow[x]*(row[x]!=lab);
            }
        }
        allComponents[0].nbPixels+=c.nbPixels;
        this->allComponents.erase(lab);
    }

    template <typename T> void drawComponent(int lab,Buffer<T> out,T outVal ){
        LabeledComponent& c=allComponents[lab];
        int y,x,width;
        t_label* row;
        T* outRow;
        width=c.getWidth();
        for(y=c.top;y<=c.bottom;y++){
            row=this->getRow(y)+c.left;
            outRow=this->getRow(y)+c.left;
            for(x=0;x<width;x++){
                outRow[x]=outVal*(row[x]==lab)+outRow[x]*(row[x]!=lab);
            }
        }
        allComponents[0].nbPixels+=c.nbPixels;
    }
    Buffer<t_uint32> getComponentIntersections(LabeledComponentSet & cs){
        int myNbComponents=this->allComponents.rbegin()->first+1;
        int csNbComponents=cs.allComponents.rbegin()->first+1;
        Buffer<t_uint32> res(csNbComponents,myNbComponents);
        int x,y,xTo,xFrom,yTo,yFrom;
        t_label * myRow;t_label *csRow;t_label curLabel;
        t_uint32 * resLine;
        for(y=0;y<res.height;y++){
            std::memset(res.getRow(y),0,res.width*sizeof(t_uint32));
        }
        for(std::map<t_label,LabeledComponent>::iterator iter=allComponents.begin();iter!=allComponents.end();++iter){
            LabeledComponent& c=iter->second;
            xFrom=c.left;xTo=c.right;
            yFrom=c.top;yTo=c.bottom;
            resLine=res.getRow(iter->first);
            for(y=yFrom;y<=yTo;y++){
                myRow=this->getRow(y);
                csRow=cs.getRow(y);
                for(x=xFrom;x<=xTo;x++){
                    if(myRow[x]){
                        resLine[csRow[x]]++;
                    }
                }
            }
        }
        return res;
    }
   // void dumpToRGBVisible(Buffer<t_uint8> R,Buffer<t_uint8> G,Buffer<t_uint8> B,bool whiteBg){
   //     t_uint8* redRow;t_uint8* greenRow;t_uint8* blueRow;
   //     t_label* myRow;
   //     int x,y;
   //     for(y=0;y<)
   // }
};


}
#endif // COMPONENT_SET_HPP
