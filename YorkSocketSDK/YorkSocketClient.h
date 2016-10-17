/*
 * YorkSocketClient.h
 *
 *  Created on: 2016年10月17日
 *      Author: yorkzero
 */

#ifndef YORKSOCKETCLIENT_H_
#define YORKSOCKETCLIENT_H_

#include"YorkNetwork.h"

namespace YorkNet {

	class YorkSocketClient : public YorkNetwork
	{
	public:
		YorkSocketClient();
		virtual ~YorkSocketClient();
	};

} /* namespace York */

#endif /* YORKSOCKETCLIENT_H_ */
