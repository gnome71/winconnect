set(backends_SRCS
	${backends_SRCS}

	core/backends/lan/server.cpp
	core/backends/lan/lanlinkprovider.cpp
	core/backends/lan/landevicelink.cpp
	core/backends/lan/lanpairinghandler.cpp
#    backends/lan/uploadjob.cpp
#    backends/lan/downloadjob.cpp
	core/backends/lan/socketlinereader.cpp

    PARENT_SCOPE
)
