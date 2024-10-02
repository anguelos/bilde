templateArgs=[{'name':'T','allowed':['bilde::t_uint8'],'isTypename':True}]
functionName='bilde::operations::essential::__subtractFrom__';

def parseFunctionArguments(funcDeclarationStr):
    funcDeclarationStr=' '.join([p for p in funcDeclarationStr.split(' ') if p !='' ])
    params=funcDeclarationStr.split('(')[1].split(')')[0].split(' ');
    params=params.split(',');
    params=[{'expression':p,'name':p.split(' ')[1],'type':p.split(' ')[0]}];
    



functionArguments=[{'type':('bilde::Buffer','T'),'isInput':True,'isOutput':False,'name':'inImg','isTemplate':True},
{'type':('Buffer','T'),'isInput':False,'isOutput':True,'name':'out','isTemplate':True},
{'type':('T',),'isInput':True,'isOutput':False,'name':'inVal','isTemplate':True}];


inputArguments=[a for a in functionArguments if a['isInput']];
outPutArguments=[a for a in functionArguments if a['isOutput']];

mexTypeNames=['bilde::t_real64':'mxDOUBLE_CLASS',
              'bilde::t_real32':'mxSINGLE_CLASS',
              'bilde::t_uint8':'mxUINT8_CLASS',
              'bilde::t_int8':'mxINT8_CLASS',
              'bilde::t_uint16':'mxUINT16_CLASS',
              'bilde::t_int16':'mxINT16_CLASS',
              'bilde::t_uint32':'mxUINT32_CLASS',
              'bilde::t_int32':'mxINT32_CLASS',
              'bilde::t_uint64':'mxUINT64_CLASS',
              'bilde::t_int64':'mxINT64_CLASS'];
for k in mexTypeNames.keys():
    mexTypeNames[k.split('::')[1]]=mexTypeNames[k]

def functionIvocation(functionName,parameters,templateInstatiations):
    res=functionName+'('
    p=''
    if parameters[0]['isTemplate']:
        p=templateInstatiations[parameters[0]['type'][-1]];
    else:
        p=parameters[0]['type'][-1];
    if len(parameters[0]['type'])>1:
        p=parameters[0]['type'][0]+'<'+p+'>'
    res=res+p
    for k =1:len(parameters):
        p=''
        if parameters[k]['isTemplate']:
            p=templateInstatiations[parameters[k]['type'][-1]];
        else:
            p=parameters[k]['type'][-1];
        if len(parameters[k]['type'])>1:
            p=parameters[k]['type'][0]+'<'+p+'>'
        res=res+','+p
    res=res+');'
    return res


def templateMultiplexing(functionName,parameters,templateInstatiations):
    res=''




initStr="""#include <vector>
#include "mex.h"
    
extern void _main();
    
mxGetNumberOfElements


bool argValidScalar(const mxArray* mxPtr){
    int mxClass=mxGetClassID(mxPtr);
    bool goodClass=(mxClass=="""+ mexTypeNames[validTypes[0]]+')'+[' && (mxClass=='+mexTypeNames[t]+')' for t in validTypes[1:]] +""";
    return  (mxGetNumberOfElements(mxPtr)==1 )&& goodClass;
}

bool argValid2d(const mxArray* mxPtr){
    int dimCounter=0;
    for(int k=0;k<mxGetNumberOfDimensions(mxPtr1);k++){
        dimCounter+=(mxGetDimensions(mxPtr1)[k]>1);
    }
    int mxClass=mxGetClassID(mxPtr);
    bool goodClass=(mxClass=="""+ mexTypeNames[validTypes[0]]+')'+[' && (mxClass=='+mexTypeNames[t]+')' for t in validTypes[1:]] +""";
    return (dimCounter==2 || dimCounter==3) && goodClass;
}
    
bool argsEqual(const mxArray* mxPtr1,const mxArray* mxPtr2){
    bool res=mxGetNumberOfDimensions(mxPtr1)==mxGetNumberOfDimensions(mxPtr1);
    for(int k=0;res && k<mxGetNumberOfDimensions(mxPtr1);k++){
            res=res && mxGetDimensions(mxPtr1)[k]==mxGetDimensions(mxPtr1)[k];
        res=res && mxGetClassID(mxPtr1)==mxGetClassID(mxPtr1);
    }
    return res;
}

void mexFunction( int nlhs, mxArray *plhs[],int nrhs, const mxArray*prhs[] ){
try{
    """++"""
}catch(const char* msg){
    std::string cppMsg="exception raised in C++ code:"+msg;
    mexErrMsgTxt(cppMsg.c_str());
}
}

"""
