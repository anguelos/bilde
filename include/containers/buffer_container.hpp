/*
 * copy_constructor.hpp
 *
 *  Created on: Jul 19, 2012
 *      Author: anguelos
 */

#ifndef COPY_CONSTRUCTOR_HPP_
#define COPY_CONSTRUCTOR_HPP_
namespace bilde{
namespace container{

template <typename PIXELT> struct ContainerWrapper<Buffer<PIXELT>,PIXELT >{
//Wrapper example.
//This wrapper is used to "wrap" a Buffer in an other Buffer
//AKA used for copy constructors

//BASIC API CLASS METHODS
	static bool isDefined(){return true;}
//BASIC API INSTANCE METHODS
	static bool canWrap(const Buffer<PIXELT>& cont){return true;}
	static bool canTakeDataOwnership(const Buffer<PIXELT>& cont){return bool(cont.__ownedstorage__);}
	static boost::shared_ptr<t_uint8> takeStorageOwnership(const Buffer<PIXELT>& cont){return cont.__ownedstorage__;}
	static boost::shared_ptr<t_uint8> deepCopyStorage(const Buffer<PIXELT>& cont){
		//dont forget to set Buffer.data to point to the apropriate place
		//linestride will always be sizeof(PIXELT)*width
		t_uint8* outdata=new t_uint8[cont.height*cont.width*sizeof(PIXELT)];
		for(int y=0;y<cont.height;y++){
			std::memcpy(outdata+cont.width*y*sizeof(PIXELT),cont.getRow(y),cont.width*sizeof(PIXELT));
		}
		return boost::shared_ptr<t_uint8>(outdata);
	}
	static const PIXELT* getFirstRow(const Buffer<PIXELT>&cont){
		return cont.getRow(0);
	}
	static PIXELT* getFirstRow(Buffer<PIXELT>&cont){
		return cont.getRow(0);
	}
	static t_sz getWidth(const Buffer<PIXELT>& cont){
		return cont.width;
	}
	static t_sz getHeight(const Buffer<PIXELT>&cont){
		return cont.height;
	}
	static t_sz getByteLinestride(const Buffer<PIXELT>&cont){
		return cont.__linestride__;
	}
};

}
}


#endif /* COPY_CONSTRUCTOR_HPP_ */
