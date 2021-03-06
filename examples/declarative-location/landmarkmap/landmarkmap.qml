/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

import Qt 4.7
import QtMobility.location 1.1
import "landmarkmapcommon" as Common
import "landmarkmapmobile" as Mobile

Item {
    id: page
    width: 320
    height: 480
    focus: true

    PositionSource {
        id: myPositionSource
        active: true
        updateInterval: 2000
        onPositionChanged: console.log("Position changed in PositionSource")
    }
    LandmarkBoxFilter {
        id: boxFilter
        topLeft: map.toCoordinate(Qt.point(0,0))
        bottomRight: map.toCoordinate(Qt.point(page.width, page.height))
    }
    LandmarkModel {
        id: landmarkModel
        autoUpdate: true
        onModelChanged: {
            console.log("Landmark model changed, landmark count: " + count)
            pinpointViewContainer.opacity = 1.0
        }
        filter: boxFilter
        limit: 50
    }
    Component {
        id: landmarkListDelegate
        Item {
            width: 200; height: 80
            Text {
                color: "white"; font.bold: true; style: Text.Raised; styleColor: "black"
                id: nameField; text: landmark.name
            }
            Text {
                color: "white"; font.bold: true; style: Text.Raised; styleColor: "black"
                anchors.top:  nameField.bottom
                id: latField; text: "    Lat: " + landmark.coordinate.latitude
            }
            Text {
                color: "white"; font.bold: true; style: Text.Raised; styleColor: "black"
                anchors.top:  latField.bottom
                id: lonField; text: "    Lon: " + landmark.coordinate.longitude
            }
        }
    }

    Component {
        id: landmarkMapDelegate
        Item {
            width: 50; height: 20
            x: map.toScreenPosition(landmark.coordinate).x
            y: map.toScreenPosition(landmark.coordinate).y
            Image {
                id: landmarkIcon
                source: "landmarkmapmobile/images/landmarkstar.png"
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        console.log("Landmark clicked, setting visible: " + landmark.name)
                        landmarkNameText.visible = true
                    }
                }
            }
            Text {
                id: landmarkNameText
                anchors.top: landmarkIcon.bottom
                text:  landmark.name
                visible: false
            }
        }
    }

    //![Category model]
    LandmarkCategoryModel {
        id: landmarkCategoryModel
        autoUpdate: true
        onModelChanged: console.log("Category model changed, category count: " + count)
    }

    Component {
        id: landmarkCategoryListDelegate
        Item {
            width: 200; height: 50
            Text {
                color: "white"; font.bold: true; style: Text.Raised; styleColor: "black"
                id: nameField; text: category.name
            }
        }
    }
    //![Category model]

    Mobile.TitleBar { id: titleBar; z: 5; width: parent.width - statusBar.width; height: 40; opacity: 0.8 }
    Mobile.StatusBar { id: statusBar; z: 6; width: 80; height: titleBar.height; opacity: titleBar.opacity; anchors.right: parent.right}

    Rectangle {
        id: dataArea
        anchors.top: titleBar.bottom
        anchors.bottom: toolbar1.bottom
        width: parent.width
        color: "#343434"
        //height: toolbar1.y - titleBar.height
        Image { source: "landmarkmapmobile/images/stripes.png"; fillMode: Image.Tile; anchors.fill: parent; opacity: 0.3 }

        MouseArea {
            anchors.fill: parent

            property bool mouseDown : false
            property int lastX : -1
            property int lastY : -1

            onPressed : {
                mouseDown = true
                // While panning, its better not to actively udpate the model
                // as it results in poor performance. Instead set opacity to make
                // it more obvious that the landmark positions are not valid.
                landmarkModel.autoUpdate = false
                pinpointViewContainer.opacity = 0.3
                lastX = mouse.x
                lastY = mouse.y
            }
            onReleased : {
                mouseDown = false
                //pinpointViewContainer.opacity = 1.0
		landmarkModel.autoUpdate = true
                landmarkModel.update()
                lastX = -1
                lastY = -1
            }
            onPositionChanged: {
                if (mouseDown) {
                    var dx = mouse.x - lastX
                    var dy = mouse.y - lastY
                    map.pan(-dx, -dy)
                    page.state = "NoFollowing"
                    lastX = mouse.x
                    lastY = mouse.y
                }
            }
            onDoubleClicked: {
                page.state = "NoFollowing"
                map.center = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                if (map.zoomLevel < map.maximumZoomLevel)
                    map.zoomLevel += 1
            }
        }

        ListView {
            id: landmarkListView;
            model: landmarkModel;
            delegate: landmarkListDelegate;
            width: parent.width - 20;
            height: parent.height -20;
            x: -(parent.width * 1.5);
            highlightFollowsCurrentItem: false
            focus: true
        }

        //![Category model]
        ListView {
            id: landmarkCategoryListView;
            model: landmarkCategoryModel;
            delegate: landmarkCategoryListDelegate;
            width: parent.width - 20;
            height: parent.height -20;
            x: -(parent.width * 1.5);
            highlightFollowsCurrentItem: false
            focus: true
        }
        //![Category model]
        Map {
            id: map
            plugin : Plugin {
                        name : "nokia"
                    }
            anchors.fill: parent
            size.width: parent.width
            size.height: parent.height
            zoomLevel: 1
            center: Coordinate {latitude: -27; longitude: 153}
            //center: myPositionSource.position.coordinate
            onZoomLevelChanged: {
                console.log("Zoom changed")
                updateFilters();
            }
            onCenterChanged: {
                console.log("Center changed")
                updateFilters();
            }
            function updateFilters () {
                // The pixel coordinates used are the map's pixel coordinates, not
                // the absolute pixel coordinates of the application. Hence e.g. (0,0) is always
                // the top left point
                var topLeftCoordinate = map.toCoordinate(Qt.point(0,0))
                var bottomRightCoordinate = map.toCoordinate(Qt.point(map.size.width, map.size.height))
                boxFilter.topLeft = topLeftCoordinate;
                boxFilter.bottomRight = bottomRightCoordinate;
            }
        } // map


        Item {
            id: pinpointViewContainer
            x: parent.x
            Repeater {
                id: landmarkPinpointView
                model: landmarkModel
                delegate: landmarkMapDelegate
            }
        }
        Mobile.Floater {
            id : dataFloater
            latitude: myPositionSource.position.coordinate.latitude
            longitude:  myPositionSource.position.coordinate.longitude
            landmarks: landmarkModel.count
            categories: landmarkCategoryModel.count
        }
    } // dataArea

    Item {
        id: sliderContainer
        anchors {bottom: toolbar1.top;}
        height:  40
        width: parent.width

        Common.Slider {
            id: zoomSlider;
            minimum: 1; maximum: 18;
            anchors {
                fill: parent
                bottomMargin: 5; rightMargin: 5; leftMargin: 5
            }
            onValueChanged: {
                console.log("Zoom slider value changed to: " + value)
                map.zoomLevel = value
            }
        }
    }

    Mobile.ToolBar {
        id: toolbar1
        opacity: titleBar.opacity
        height: 40; width: parent.width
        anchors.bottom: toolbar2.top
        z: 6
        button1Label: "map"; button2Label: "landmarks"; button3Label: "categories"
        onButton1Clicked: {
            if (page.state != "")
                page.state = ""
        }
        onButton2Clicked: {
            if (page.state != "LandmarkView")
                page.state = "LandmarkView"
        }
        onButton3Clicked: {
            if (page.state != "CategoryView")
                page.state = "CategoryView"
        }
    }

    Mobile.ToolBar {
        id: toolbar2
        opacity: toolbar1.opacity
        height: 40; width: parent.width
        anchors.bottom: parent.bottom
        z: 6
        button1Label: "nmealog.txt"; button2Label: "mylm.lmx"; button3Label: "follow me"
        button3FontColor: "pink"
        onButton1Clicked: {
            console.log("Clicked, setting nmea log as source");
            myPositionSource.nmeaSource = "nmealog.txt"
            myPositionSource.start()
        }
        onButton2Clicked: {
            console.log("Clicked, setting import file to get landmarks, count was: " + landmarkModel.count);
            landmarkModel.importFile = "mylm.lmx"
        }
        onButton3Clicked: {
            console.log("Clicked, setting map center to follow (rebind map center to myPosition");
            //if (page.state == "" || page.state == "Following")
                page.state = "Following"
                //map.center = myPositionSource.position.coordinate
        }
    }
    // states of page
    states: [State {
            name: "CategoryView"
            PropertyChanges { target: map; x: -dataArea.width }
            PropertyChanges { target: map; x: -dataArea.width }
            PropertyChanges { target: sliderContainer; x: -dataArea.width}
            PropertyChanges { target: pinpointViewContainer; x: -dataArea.width - 20;}
            PropertyChanges { target: landmarkCategoryListView; x: 0 }
        }, State {
            name: "LandmarkView"
            PropertyChanges { target: map; x: -dataArea.width }
            PropertyChanges { target: sliderContainer; x: -dataArea.width}
            PropertyChanges { target: pinpointViewContainer; x: -dataArea.width - 20;}
            PropertyChanges { target: landmarkListView; x: 0 }
        }, State {
            name:  "Following"
            PropertyChanges { target:  map; center: myPositionSource.position.coordinate}
            PropertyChanges { target: toolbar2; button3FontColor: 'grey'}
            PropertyChanges { target: toolbar2; button3Label: '(following)'}
        }, State {
            name : "NoFollowing"
            PropertyChanges { target: toolbar2; button3FontColor: 'pink'}
            PropertyChanges { target: toolbar2; button3Label: 'follow me'}
        }]
    // state-transition animations for page
    transitions: Transition {
        NumberAnimation { properties: "x"; duration: 500; easing.type: Easing.InOutQuad }
    }
} // page
