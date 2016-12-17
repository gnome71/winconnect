set(backends_SRCS
	backends/lan/server.cpp
	backends/lan/lanlinkprovider.cpp
	backends/lan/landevicelink.cpp
	backends/lan/lanpairinghandler.cpp
#    backends/lan/uploadjob.cpp
#    backends/lan/downloadjob.cpp
	backends/lan/socketlinereader.cpp

    PARENT_SCOPE
)

set(backends_HDRS
	backends/lan/server.h
	backends/lan/lanlinkprovider.h
	backends/lan/landevicelink.h
	backends/lan/lanpairinghandler.h
#    backends/lan/uploadjob.h
#    backends/lan/downloadjob.h
	backends/lan/socketlinereader.h

    PARENT_SCOPE
)
