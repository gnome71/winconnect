set(backends_SRCS
#	${backends_SRCS}

	core/backends/lan/server.cpp
	core/backends/lan/lanlinkprovider.cpp
	core/backends/lan/landevicelink.cpp
	core/backends/lan/lanpairinghandler.cpp
#    backends/lan/uploadjob.cpp
#    backends/lan/downloadjob.cpp
	core/backends/lan/socketlinereader.cpp

    PARENT_SCOPE
)

set(backends_HDRS
#	${backends_HDRS}

	core/backends/lan/server.h
	core/backends/lan/lanlinkprovider.h
	core/backends/lan/landevicelink.h
	core/backends/lan/lanpairinghandler.h
#    backends/lan/uploadjob.h
#    backends/lan/downloadjob.h
	core/backends/lan/socketlinereader.h

    PARENT_SCOPE
)
