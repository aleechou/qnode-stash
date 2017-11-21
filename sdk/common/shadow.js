qnode.ShadowObject = class {
    constructor(object) {
        this.object = object
    }

    wrap(object) {

    }

    pushToWindow(window, afterScript) {
        if (window.objId == undefined) {
            return
        }


    }
}



qnode.classes.ShadowObject = qnode.ShadowObject


var value_changeds = {}
var signed_var_id = 0

qnode.shadow = function(object) {
    for (var name in object) {
        var type = typeof object[name]
        if (type == "function")
            continue
        else if (type == "object")
            qnode.shadow(object[name])
        else {
            ((name, value) => {
                var varid = signed_var_id++;
                value_changeds[varid] = (val, from) => {
                    value = val
                }
                Object.defineProperty(object, name, {
                    get: () => value,
                    set: value_changeds[varid]
                })
            })(name, object[name])

        }
    }
}