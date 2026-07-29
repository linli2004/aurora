import QtQuick
import Aurora.Runtime 1.0

Item {
    id: root

    property int accessibilityMode: AuroraTypes.AccessibilityNormal
    property int qualityMode: AuroraTypes.Balanced

    property string outgoingTitle: ""
    property string outgoingArtist: ""
    property url outgoingArtworkSource
    property color outgoingColor: AuroraTokens.coolAccent

    property string incomingTitle: ""
    property string incomingArtist: ""
    property url incomingArtworkSource
    property color incomingColor: AuroraTokens.coolAccent

    // Exact destination of the real Music Space crystal. Both overlay
    // identities use this geometry so the final handoff cannot resize or jump.
    property rect targetRect: Qt.rect(0, 0, 320, 320)

    readonly property bool reducedMotion:
        accessibilityMode === AuroraTypes.AccessibilityReducedMotion
    readonly property bool running: transitionActive
    readonly property int direction: transitionDirection

    // v0.5.3 deliberately slows the transition down. The motion is no longer
    // a quick swap: the current identity first enlarges, then rolls away while
    // shrinking; the next identity grows progressively from the opposite edge.
    readonly property real transitionDuration: reducedMotion ? 280 : 1450
    readonly property real centerOffset: Math.min(root.width * 0.22, 300)
    readonly property real travelDistance: reducedMotion
                                                ? Math.min(32, root.width * 0.03)
                                                : Math.max(root.centerOffset * 1.85,
                                                           root.targetRect.width * 1.55)
    readonly property real arcLift: reducedMotion ? 0 : Math.min(15, root.height * 0.018)
    readonly property real arcDrop: reducedMotion ? 0 : Math.min(46, root.height * 0.055)
    readonly property real outgoingPeakScale: reducedMotion ? 1.0 : 1.11
    readonly property real edgeScale: reducedMotion ? 1.0 : 0.34
    readonly property real incomingPeakScale: reducedMotion ? 1.0 : 1.04
    readonly property real carouselRotation: reducedMotion ? 0.0 : 5.5

    signal switchRequested(int direction)
    signal landingStarted()
    signal completed()

    property int transitionDirection: 1
    property bool transitionActive: false

    function start(directionValue) {
        if (transitionActive)
            return

        transitionDirection = directionValue < 0 ? -1 : 1
        transitionActive = true
        transition.restart()
    }

    visible: transitionActive
    enabled: visible
    z: 200
    clip: true

    // A transparent veil protects readability while keeping the original
    // atmosphere visible. It never becomes an opaque page replacement.
    Rectangle {
        id: veil
        anchors.fill: parent
        color: AuroraTokens.windowBackground
        opacity: 0.0
    }

    Rectangle {
        id: upperFlow
        width: root.width * 1.34
        height: Math.max(140, root.height * 0.20)
        radius: height / 2
        y: root.height * 0.12
        color: Qt.rgba(root.outgoingColor.r,
                       root.outgoingColor.g,
                       root.outgoingColor.b, 0.11)
        opacity: root.reducedMotion ? 0.0 : 1.0
        transformOrigin: Item.Center
    }

    Rectangle {
        id: lowerFlow
        width: root.width * 1.28
        height: Math.max(125, root.height * 0.17)
        radius: height / 2
        y: root.height * 0.60
        color: Qt.rgba(root.incomingColor.r,
                       root.incomingColor.g,
                       root.incomingColor.b, 0.10)
        opacity: root.reducedMotion ? 0.0 : 1.0
        transformOrigin: Item.Center
    }

    Rectangle {
        id: centerBloom
        anchors.centerIn: parent
        width: Math.min(root.width, root.height) * 0.40
        height: width
        radius: width / 2
        color: Qt.rgba(root.incomingColor.r,
                       root.incomingColor.g,
                       root.incomingColor.b,
                       root.reducedMotion ? 0.025 : 0.055)
        opacity: 0.0
        scale: 0.90
    }

    // Only music identity is duplicated in the overlay. Text and controls
    // belong to the real destination and appear gradually during landing.
    Item {
        id: outgoingGroup
        width: Math.max(1, root.targetRect.width)
        height: Math.max(1, root.targetRect.height)
        transformOrigin: Item.Center
        layer.enabled: root.running && !root.reducedMotion
        layer.smooth: true

        AuroraCrystal {
            anchors.fill: parent
            crystalSize: parent.width
            title: root.outgoingTitle
            artist: root.outgoingArtist
            artworkSource: root.outgoingArtworkSource
            colorSignature: root.outgoingColor
            experienceState: AuroraTypes.CrystalTransitioning
            context: AuroraTypes.MusicSpace
            accessibilityMode: root.accessibilityMode
            qualityMode: root.qualityMode
            heroMode: true
            mangaMode: true
        }
    }

    Item {
        id: incomingGroup
        width: Math.max(1, root.targetRect.width)
        height: Math.max(1, root.targetRect.height)
        transformOrigin: Item.Center
        layer.enabled: root.running && !root.reducedMotion
        layer.smooth: true

        AuroraCrystal {
            anchors.fill: parent
            crystalSize: parent.width
            title: root.incomingTitle
            artist: root.incomingArtist
            artworkSource: root.incomingArtworkSource
            colorSignature: root.incomingColor
            experienceState: AuroraTypes.CrystalTransitioning
            context: AuroraTypes.MusicSpace
            accessibilityMode: root.accessibilityMode
            qualityMode: root.qualityMode
            heroMode: true
            mangaMode: true
        }
    }

    SequentialAnimation {
        id: transition

        ScriptAction {
            script: {
                veil.opacity = 0.0
                upperFlow.x = -root.width * 0.18
                upperFlow.rotation = -13
                lowerFlow.x = -root.width * 0.02
                lowerFlow.rotation = 16
                centerBloom.opacity = 0.0
                centerBloom.scale = 0.90

                outgoingGroup.x = root.targetRect.x
                outgoingGroup.y = root.targetRect.y
                outgoingGroup.opacity = 1.0
                outgoingGroup.scale = 1.0
                outgoingGroup.rotation = 0.0

                incomingGroup.x = root.targetRect.x
                                + root.transitionDirection * root.travelDistance
                incomingGroup.y = root.targetRect.y + root.arcDrop
                incomingGroup.opacity = 0.0
                incomingGroup.scale = root.edgeScale
                incomingGroup.rotation = root.transitionDirection * root.carouselRotation
            }
        }

        // One continuous carousel timeline:
        // 0–18%   current identity enlarges in place;
        // 18–82%  current identity rolls away and shrinks while the next grows;
        // 70–100% real Music Space content fades in under the arriving identity;
        // 86–100% overlay dissolves only after the destination is already visible.
        ParallelAnimation {
            SequentialAnimation {
                OpacityAnimator {
                    target: veil
                    from: 0.0
                    to: root.reducedMotion ? 0.13 : 0.20
                    duration: root.transitionDuration * 0.28
                    easing.type: Easing.OutCubic
                }
                PauseAnimation { duration: root.transitionDuration * 0.38 }
                OpacityAnimator {
                    target: veil
                    to: 0.0
                    duration: root.transitionDuration * 0.34
                    easing.type: Easing.OutQuint
                }
            }

            // Current song: visibly enlarge before it begins moving. It follows
            // a shallow carousel arc, progressively shrinks and only then fades.
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.16 }
                XAnimator {
                    target: outgoingGroup
                    from: root.targetRect.x
                    to: root.targetRect.x
                        - root.transitionDirection * root.travelDistance
                    duration: root.transitionDuration * 0.84
                    easing.type: Easing.InOutCubic
                }
            }
            SequentialAnimation {
                YAnimator {
                    target: outgoingGroup
                    from: root.targetRect.y
                    to: root.targetRect.y - root.arcLift
                    duration: root.transitionDuration * 0.18
                    easing.type: Easing.OutCubic
                }
                YAnimator {
                    target: outgoingGroup
                    to: root.targetRect.y + root.arcDrop
                    duration: root.transitionDuration * 0.70
                    easing.type: Easing.InOutCubic
                }
                PauseAnimation { duration: root.transitionDuration * 0.12 }
            }
            SequentialAnimation {
                ScaleAnimator {
                    target: outgoingGroup
                    from: 1.0
                    to: root.outgoingPeakScale
                    duration: root.transitionDuration * 0.18
                    easing.type: Easing.OutCubic
                }
                ScaleAnimator {
                    target: outgoingGroup
                    to: root.edgeScale
                    duration: root.transitionDuration * 0.70
                    easing.type: Easing.InOutCubic
                }
                PauseAnimation { duration: root.transitionDuration * 0.12 }
            }
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.16 }
                RotationAnimator {
                    target: outgoingGroup
                    from: 0.0
                    to: -root.transitionDirection * root.carouselRotation
                    duration: root.transitionDuration * 0.84
                    easing.type: Easing.InOutCubic
                }
            }
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.20 }
                OpacityAnimator {
                    target: outgoingGroup
                    from: 1.0
                    to: 0.0
                    duration: root.transitionDuration * 0.66
                    easing.type: Easing.InCubic
                }
                PauseAnimation { duration: root.transitionDuration * 0.14 }
            }

            // Next song: begin nearly invisible at the opposite edge, then grow
            // continuously as it travels toward the playback position.
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.18 }
                XAnimator {
                    target: incomingGroup
                    from: root.targetRect.x
                          + root.transitionDirection * root.travelDistance
                    to: root.targetRect.x
                        - root.transitionDirection * (root.reducedMotion ? 0 : 12)
                    duration: root.transitionDuration * 0.68
                    easing.type: Easing.OutCubic
                }
                XAnimator {
                    target: incomingGroup
                    to: root.targetRect.x
                    duration: root.transitionDuration * 0.14
                    easing.type: Easing.OutQuint
                }
            }
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.18 }
                YAnimator {
                    target: incomingGroup
                    from: root.targetRect.y + root.arcDrop
                    to: root.targetRect.y - root.arcLift * 0.55
                    duration: root.transitionDuration * 0.68
                    easing.type: Easing.OutCubic
                }
                YAnimator {
                    target: incomingGroup
                    to: root.targetRect.y
                    duration: root.transitionDuration * 0.14
                    easing.type: Easing.OutQuint
                }
            }
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.18 }
                ScaleAnimator {
                    target: incomingGroup
                    from: root.edgeScale
                    to: root.incomingPeakScale
                    duration: root.transitionDuration * 0.68
                    easing.type: Easing.OutCubic
                }
                ScaleAnimator {
                    target: incomingGroup
                    to: 1.0
                    duration: root.transitionDuration * 0.14
                    easing.type: Easing.OutQuint
                }
            }
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.18 }
                RotationAnimator {
                    target: incomingGroup
                    from: root.transitionDirection * root.carouselRotation
                    to: -root.transitionDirection * (root.reducedMotion ? 0 : 0.45)
                    duration: root.transitionDuration * 0.68
                    easing.type: Easing.OutCubic
                }
                RotationAnimator {
                    target: incomingGroup
                    to: 0.0
                    duration: root.transitionDuration * 0.14
                    easing.type: Easing.OutQuint
                }
            }
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.13 }
                OpacityAnimator {
                    target: incomingGroup
                    from: 0.0
                    to: 1.0
                    duration: root.transitionDuration * 0.52
                    easing.type: Easing.OutCubic
                }
                PauseAnimation { duration: root.transitionDuration * 0.19 }
                OpacityAnimator {
                    target: incomingGroup
                    from: 1.0
                    to: 0.0
                    duration: root.transitionDuration * 0.16
                    easing.type: Easing.OutQuint
                }
            }

            // Switch only the lightweight queue identity here. The player
            // source is committed by MusicSpace after the transition frame
            // has completed, so QMediaPlayer loading cannot stall the motion.
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.18 }
                ScriptAction { script: root.switchRequested(root.transitionDirection) }
            }

            // The real player begins appearing before the overlay disappears.
            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.70 }
                ScriptAction { script: root.landingStarted() }
            }

            XAnimator {
                target: upperFlow
                from: -root.width * 0.18
                to: -root.width * 0.18
                    + root.transitionDirection * root.width * 0.28
                duration: root.transitionDuration
                easing.type: Easing.InOutCubic
            }
            RotationAnimator {
                target: upperFlow
                from: -13
                to: -4 + root.transitionDirection * 4
                duration: root.transitionDuration
                easing.type: Easing.InOutCubic
            }

            XAnimator {
                target: lowerFlow
                from: -root.width * 0.02
                to: -root.width * 0.02
                    - root.transitionDirection * root.width * 0.24
                duration: root.transitionDuration
                easing.type: Easing.InOutCubic
            }
            RotationAnimator {
                target: lowerFlow
                from: 16
                to: 5 - root.transitionDirection * 4
                duration: root.transitionDuration
                easing.type: Easing.InOutCubic
            }

            SequentialAnimation {
                PauseAnimation { duration: root.transitionDuration * 0.24 }
                ParallelAnimation {
                    OpacityAnimator {
                        target: centerBloom
                        from: 0.0
                        to: 1.0
                        duration: root.transitionDuration * 0.26
                        easing.type: Easing.OutCubic
                    }
                    ScaleAnimator {
                        target: centerBloom
                        from: 0.90
                        to: 1.06
                        duration: root.transitionDuration * 0.44
                        easing.type: Easing.OutCubic
                    }
                }
                PauseAnimation { duration: root.transitionDuration * 0.10 }
                OpacityAnimator {
                    target: centerBloom
                    to: 0.0
                    duration: root.transitionDuration * 0.26
                    easing.type: Easing.OutQuint
                }
            }
        }

        ScriptAction {
            script: {
                root.transitionActive = false
                root.completed()
            }
        }
    }
}
