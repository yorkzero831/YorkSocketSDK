/*
 * YorkNetwork.cpp
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#include "YorkNetwork.h"


namespace YorkNet {

	YorkNetwork::YorkNetwork() {
		// TODO Auto-generated constructor stub

	}

	YorkNetwork::~YorkNetwork() {
		// TODO Auto-generated destructor stub
	}

	void YorkNetwork::ShowErrorMessage(ErrorMessage message)
	{
		cout << message.errorWords << " WITH ERROMESSAGE : " << message.details<<endl;
	}

} /* namespace York */
