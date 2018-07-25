#!/opt/local/bin/python2.7

#PRIMARY SOURCE http://eli.thegreenplace.net/2011/07/03/parsing-c-in-python-with-clang/
import commands
import sys
import clang.cindex
print(commands.getoutput('pwd'))


class BildeParameter:
    def __init__(self,tokenList,templateArgList):
        constTokenList=[]
        for t in tokenList:
            if t=='const':
                constTokenList.append(' '+t+' ')
            else:
                constTokenList.append(t)
        self.tokens=tokenList
        self.name=constTokenList[-1]
        self.typeName=''.join(constTokenList[:-1])

    def instanciateType(self,templateParameterName,instantiationName):
    pass

    def renderWrapperDeclaration(self,uninstantiatedTemplateMap):
        pass

    def renderInvocation(self,instantiatedTemplateMap):
        pass

    def getTemlateArgs()
        pass


class BildeFunction:
    @staticmethod
    def getFunctionParameters(clangFuncObj):
        for clangFuncObj
    @staticmethod
    def __getAllBildeFunctions__(srcRootFileName):
        clang.cindex.Config.set_library_file('/opt/local/libexec/llvm-3.5/lib/libclang.dylib')#this will have to change for every system manually or be incorporated to cmake
        index = clang.cindex.Index.create()
        tu = index.parse(srcRootFileName)
        lev0=tu.cursor
        res=[]
        for lev1 in list(lev0.get_children()):
            if lev1.kind==clang.cindex.CursorKind.NAMESPACE and lev1.spelling=='bilde':
                for lev2 in list(lev1.get_children()):
                    if lev2.kind==clang.cindex.CursorKind.NAMESPACE and lev2.spelling=='operations':
                        for lev3 in list(lev2.get_children()):
                            if lev3.kind==clang.cindex.CursorKind.NAMESPACE and not(lev3.spelling.startswith('__')) and not(lev3.spelling.endswith('__')):
                                for lev4 in list(lev3.get_children()):
                                    if lev4.kind==clang.cindex.CursorKind.FUNCTION_TEMPLATE or lev4.kind==clang.cindex.CursorKind.FUNCTION_DECL:
                                        print(lev4.spelling)
                                        res.append((lev4,'bilde::operations::'+lev3.spelling))
        return res

    @staticmethod
    def LoadAllFunctions(scrRootFname):
        return [BildeFunction(clangObj) for clangObj in BiledFunction.getAllBildeFunctions(srcRootFileName)]

    def __str__():
        return 'Name: '+self.getName+'\n'

    def __init__(self,clangObj):
        self.clangObj=clangObj
        self.name=self.clangObj.spelling
        self.returnType=self.clangObj.returnType
        self.tokens=list(self.clangObj.get_tokens())
        pass

    def getName(self):
        return self.clangObj.spelling




flist=BildeFunction.__getAllBildeFunctions__('../include/bilde.hpp')
print flist[41][0].kind
