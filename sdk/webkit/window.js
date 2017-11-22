$window.bridgeEvent = new EventEmitter()

for (var methodName in ['on', 'once']) {;
    ((methodName) => {
        $window[methodName] = function() {
            return $window.bridgeEvent[methodName].apply($window.bridgeEvent, arguments)
        }
    })(methodName)
}


$window.emit = function(eventName, ...argv) {
    // 触发本地监听
    $window.bridgeEvent.emit(eventName, ...argv)

    // 向父线程触发事件
    qnode.api.run(qnode.api.parentThreadId, () => {
        qnode.window.eventEmit(objId, eventName, argv)
    }, { objId: qnode.api.threadId, eventName, argv })
}