#ifndef BILDE_DEBUG
#define BILDE_DEBUG

namespace bilde{
namespace debug{


#ifdef __OPENCV_HIGHGUI_HPP__
template <typename P> void __showBufferSync__(Buffer<P> b,std::string winName="Debug Window",char pauseUntil='n'){
    int k=-1;
    std::cerr<<"Press '"<<char(pauseUntil)<<"' to continue\n";
    while(k!=pauseUntil){
        cv::namedWindow(winName);
        cv::imshow(winName,cv::Mat(b.height, b.getPixelLinestride(), cv::DataType<P>::type, b.__data__, CV_AUTOSTEP));
        if(pauseUntil!='\0'){
            k=cv::waitKey(1000);
        }else{
            k=pauseUntil;
        }
    }
}

template <typename P> void __showBufferAsync__(Buffer<P> b,std::string winName="Debug Window"){
    cv::namedWindow(winName);
    cv::imshow(winName,cv::Mat(b.height, b.getPixelLinestride(), cv::DataType<P>::type, b.__data__, CV_AUTOSTEP));
}

template <typename P> void __dumpBuffer__(Buffer<P> b,std::string fname){
    cv::imwrite(fname,cv::Mat(b.height, b.getPixelLinestride(), cv::DataType<P>::type, b.__data__, CV_AUTOSTEP));
}

#else //NOHIGHGUIAVAILABLE
template <typename P> void __showBufferSync__(Buffer<P> b,std::string ="Debug Window",char pauseUntil='n'){
    if(bilde::util::Args().verboseLevel>0){
        std::cerr<<"Cannot show buffer!";
    }
}
template <typename P> void __showBufferAsync__(Buffer<P> b,std::string ="Debug Window",char pauseUntil='n'){
    if(bilde::util::Args().verboseLevel>0){
        std::cerr<<"Cannot show buffer!";
    }
}

template <typename P> void __dumpBuffer__(Buffer<P> b,std::string fname){
    if(bilde::util::Args().verboseLevel>0){
        std::cerr<<"Cannot dump buffer!";
    }
}

#endif //ifdef __OPENCV_HIGHGUI_HPP__



struct __BufferDebugger__{
    int dumpCounter;
    std::string dumpPrefix;
    bool dumpShowFrames;
    std::ostream& out;
    std::stringstream null;
    __BufferDebugger__():
            dumpCounter(0),dumpPrefix("__DBG_"),dumpShowFrames(false),out(std::cerr){
        null.setstate(std::ios_base::badbit);
    }
    std::ostream& operator()(int vl){
        if(bilde::util::Args().verboseLevel>=vl){
            return out;
        }else{
            return null;
        }
    }
    template<typename P> void show(std::initializer_list<Buffer<P> > bList,std::string name=""){
        int k=0;
        if (name==""){
            name="Debug Window";
        }
        for(auto b=bList.begin();b!=bList.end();++b){
            if(dumpShowFrames){
                __dumpBuffer__<P>(*b,this->dumpPrefix+std::to_string(dumpCounter)+".png");
                dumpCounter++;
            }
            if(k==bList.size()-1){
                __showBufferSync__(*b,name+std::to_string(k));
            }else{
                __showBufferAsync__(*b,name+std::to_string(k));
            }
            k++;
        }
    }
    template<typename P> void show(Buffer<P> b,std::string name=""){
            if(dumpShowFrames){
                __dumpBuffer__<P>(b,this->dumpPrefix+std::to_string(dumpCounter)+".png");
                dumpCounter++;
            }
            if (name==""){
                name="Debug Window";
            }
            __showBufferSync__(b,name);
    }
    template<typename P> void dump(Buffer<P> b,std::string fname=""){
        if(fname==""){
            fname=this->dumpPrefix+std::to_string(dumpCounter)+".png";
        }
        __dumpBuffer__<P>(b,fname);
        dumpCounter++;
    }
    template<typename P> void dump(std::initializer_list<Buffer<P> > bList,std::string fname=""){
        if(fname==""){
            fname=this->dumpPrefix+std::to_string(dumpCounter);
        }
        int counter=0;
        for(auto b=bList.begin();b!=bList.end();++b){
            __dumpBuffer__<P>(*b,fname+"_"+std::to_string(counter)+".png");
            counter++;
        }
        dumpCounter++;
    }
};


static __BufferDebugger__& __sigletonDebugger__(){
    static __BufferDebugger__ b;
    return b;
}

std::ostream& dbgMsg(int vl){
    static  __BufferDebugger__& dbg=__sigletonDebugger__();
    if(bilde::util::Args().verboseLevel>5){
        return dbg(vl)<<bilde::util::globalTimer().getMsec()<<"\t:";
    }else{
        return dbg(vl);
    }
}

template <typename P> void dbgShow(int vl,Buffer<P>b,std::string name=""){
    static  __BufferDebugger__& dbg=__sigletonDebugger__();
    if(bilde::util::Args().verboseLevel>=vl){
        dbg.show(b,name);
    }
}

template <typename P> void dbgShow(int vl,std::initializer_list<Buffer<P> >b,std::string name=""){
    static  __BufferDebugger__& dbg=__sigletonDebugger__();
    if(bilde::util::Args().verboseLevel>=vl){
        dbg.show(b,name);
    }
}


}
}

#endif //ifndef BILDE_CV2_DEBUG

