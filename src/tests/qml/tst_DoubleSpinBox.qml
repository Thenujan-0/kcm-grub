import QtQuick 2.0
import QtTest 1.2

import "../../package/contents/ui"

DoubleSpinBox {
    id: dsb
    property var value: 0
    text: value
    onIncrease: function(){
        value += 1
    }
    onDecrease: function(){
        value -= 1
    }

    onEditFinished: {
        value = Number(text)
    }
    TestCase {
        name: "DoubleSpinBox Tests"
        when: windowShown

        function test_add() {
            mouseClick(dsb,dsb.width-50)
            compare(dsb.text,"1")
        }

        function test_reduce() {
            mouseClick(dsb,dsb.width-25)
            compare(dsb.text,"0")
        }

        function test_input_invalid(){
            //Letters are not insertable
            dsb.inserted(1,"abc")
            compare(dsb.text,"1")

            //Negative numbers are not insertable
            dsb.inserted(0,"-1")
            compare(dsb.text,"1")
        }
        
    }
}
