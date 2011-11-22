/**
 * @file cxx/ip/ip/ipHistoEqual.h
 * @date Tue Jan 18 17:07:26 2011 +0100
 * @author André Anjos <andre.anjos@idiap.ch>
 *
 * Copyright (C) 2011 Idiap Reasearch Institute, Martigny, Switzerland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _TORCH5SPRO_IP_HISTO_EQUAL_H_
#define _TORCH5SPRO_IP_HISTO_EQUAL_H_

#include "ip/ipCore.h"

namespace Torch {

	/** This class is designed to enhance an image using histogram equalisation

	    \verbatim

	         +---+          +--------------+         +---+
	         |xxx|	        |              |         |XXX|
		 |xxx|   ---->  | ipHistoEqual | ---->   |XXX|
	         |xxx|          |              |         |XXX|
	         +---+          +--------------+         +---+
	
	    \endverbatim
	    
	    @author Sebastien Marcel (marcel@idiap.ch)
	    @author Laurent El Shafey (laurent.el-shafey@idiap.ch)
	    @version 2.0
	    \date
	    @since 2.0
	*/

	class ipHistoEqual : public ipCore
	{
	public:

		/// constructor
	    	ipHistoEqual();

		/// destructor
		virtual ~ipHistoEqual();
	
	protected:

		////////////////////////////////////////////////////////////////////////////////////////////

		/// Check if the input tensor has the right dimensions and type - overriden
		virtual bool            checkInput(const Tensor& input) const;
		
		/// Allocate (if needed) the output tensors given the input tensor dimensions - overriden
		virtual bool            allocateOutput(const Tensor& input);
		
		/// Process some input tensor (the input is checked, the outputs are allocated) - overriden
		virtual bool            processInput(const Tensor& input);

		////////////////////////////////////////////////////////////////////////////////////////////
		
	private:

	};

}

#endif

