;
(async() => {

    var win = await qnode.window.create()

    win.setVisible(true)
    win.load("http://www.baidu.com")

    console.log(win.qtmeta)


    var inspector = await qnode.window.create()
    inspector.show()
    inspector.load("http://127.0.0.1:17135")

})()