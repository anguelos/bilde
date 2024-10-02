#!/opt/local/bin/python3.4

functions=eval(open('declarations.py').read())


class FunctionParameter:
    otherParameters={}
    def __init__(self,declStr,sizeAs==''):
        self.typeStr=' '.join(declStr.split(' ')[:-1])
        self.shortTstr=self.typeStr.replace(' ','').replace('bilde::','');
        #self.shortTstr=self.shortTstr.replace('unsignedchar','t_uint8').replace('unsignedshort','t_uint16').replace('unsignedshortint','t_uint16').replace
        self.nameStr=declStr.split(' ')[-1]
        if sizeAs=='':
            self.sameSizeAsNamed=self.nameStr()
        else:
            self.sameSizeAsNamed=sizeAs
        otherParameters[self.nameStr]=self
    def clone(self,declStr=''):
        """must be reimplemented for derived classes"""
        if declStr=='':
            return FunctionParameter(self.typeStr+' '+self.nameStr)
        return FunctionParameter(declStr)
    def getInputData(self):
        """Method that return the sourcecode that extracts the parameter"""
        raise('pure virtual method. This should not happen')
    def getTypeValidation(self):
        return 'if(true){}else{throw "bad type given, expected '+self.shortTstr+'";}\n'
    def getSizeValidation(self):
        if self.nameStr!=self.sameSizeAsNamed:
            return 'if('+self.name+'.width!='+self.sameSizeAsNamed+'.width || '+
                self.name+'.height!='+self.sameSizeAsNamed+'.height){\nthrow "'+
                self.name+' and '+self.sameSizeAsNamed+' must have the same size";\n}'
    def instanciateAs(self,asType):
            if shortTstr=='T':
                return self.clone(asType+' '+self.declStr)
            if shortTstr=='Buffer<T>':
                return self.clone('Buffer<'asType+'> '+self.declStr)
            return self.clone()
    def isInput(self):
        return self.nameStr.startswith('in')
    def isOutput(self):
        return self.nameStr.startswith('out') or self.nameStr.startswith('inout')

class FunctionDeclaration:
    def __init__(self,lst):
        declStr=lst[0]
        declStr=' '.join([d for d in declStr.split(' ') if d!=''])
        self.paramStrings=lst[1:]
        self.updateParameters()
        self.fullName=declStr.split('(')[0].split(' ')[-1]

    def updateParameters(self):
        self.parameters=[FunctionParameter(pStr) for pStr in self.paramStrings]
    
    def getSimpleName(self):
        return self.fullName.split('::')[-1]

    def getNamespace(self):
        return '::'.join(self.fullName.split('::')[:-1])

class MexParameter(FunctionParameter):
    numericParameterCode=['t_real64':'mxDOUBLE_CLASS','t_real32':'mxSINGLE_CLASS',
                        't_uint8':'mxUINT8_CLASS','t_int8':'mxINT8_CLASS',
                                       't_uint16':'mxUINT16_CLASS',
                                       't_int16':'mxINT16_CLASS',
                                       't_uint32':'mxUINT32_CLASS',
                                       't_int32':'mxINT32_CLASS',
                                       't_uint64':'mxUINT64_CLASS',
                                       't_int64':'mxINT64_CLASS'];
    def __init__(self,declStr,functionParser):
        FunctionParameter.__init__(self,declStr)
        self.functionParser=functionParser
        if self.isInput:
            self.inputPosition=functionParameter.nbInput=functionParameter.nbInput+1
        else:
            self.inputPosition=-1
    def clone(self,declStr=''):
        if declStr=='':
            return MexParameter(self.typeStr+' '+self.nameStr,self.paramNum)
        else:
            return MexParameter(declStr)
    def getInputData(self):
        return '(prhs['+str(self.paramNum)+'])'
    def getTypeValidation(self):
        return