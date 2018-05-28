#~/bin/bash

PNGS_DIR=./pngs

MYNAME=$(basename $0)

###
# Function to export PNGs at various horn angles using OpenSCAD command line
# args. Note that the version of OpenSCAD used here needs to be the 2015.03-1
# release or later.
###
function exportPNGs() {
    # Defaults are to invert the angle, and generate PNGs at 5° increments from
    # -90° to +90° and dump then in the PNGS_DIR
    inverted="true"
    min=-90
    max=90
    step=5

    outDir=${PNGS_DIR}
    outBase=angle

    mkdir -p ${outDir}

    for a in $(seq $min $step $max); do
        outF=$(printf "${outDir}/${outBase}%+02.2d.png" $a)
        openscad -o "$outF" servoSim.scad --colorscheme=BeforeDawn \
                 --camera=2,12,0,27,0,48,130 --imgsize=250,300 \
                 -D angle=$a -D inverted=$inverted
    done
}

###
# Function to create a set of frames from the PNGs in PNGS_DIR, and then create
# a gif animation from these. The frames are created as symlinks in a temp dir
# based on the offset and amplitude required for the animation.
#
# Arguments:
#  offset: the offest angle - should be a multiple of 5° between -90° and +90°
#  amplitude: the animation amplitude between -90° and +90° as a multiple of 5°
###
function makeAnimation() {
    offs=$1
    ampl=$2
    FRAMESDIR=_frames_

    beg=$((0 - $ampl + $offs))
    stp=$(($beg + $ampl*2))
    rm -rf $FRAMESDIR
    mkdir $FRAMESDIR

    # Create frame symlinks for first half cycle
    c=0;
    for f in $(seq $beg 5 $stp); do 
        src=$(printf "$(realpath ${PNGS_DIR})/angle%+02.2d.png" $f)
        tgt=$(printf "${FRAMESDIR}/frame-%02.2d.png" $c)
        c=$((c + 1))
        ln -s $src $tgt
    done
    # Create frame symlinks for second half cycle
    for f in $(seq $stp -5 $beg); do 
        src=$(printf "$(realpath ${PNGS_DIR})/angle%+02.2d.png" $f)
        tgt=$(printf "${FRAMESDIR}/frame-%02.2d.png" $c)
        c=$((c + 1))
        ln -s $src $tgt
    done
    # Create animation - not sure what the delay relationship is, but this
    # seems to be general enough
    convert "${FRAMESDIR}/*.png" -set delay 1x30 servo-A${ampl}O${offs}.gif

    rm -rf $FRAMESDIR
}

function showHelp() {
    cat - << _HEREDOC_

Usage:

$MYNAME pngs

or

$MYNAME offset amplitude

The first format will generate a set of PNGs with the servo horn at 5°
increments from -90° to +90°

The second format will use the generated PNGs to create an animation with
offset and amplitude as supplied. All angles should be a multiple of 5°.

_HEREDOC_
}

# We need args
[ $# -lt 1 ] && showHelp && exit 1

if [ "$1" == "pngs" ]; then
   exportPNGs
else
    offs=$1
    ampl=$2
    # We have the first arg, but validate the second
    [ "$ampl" == "" ] && showHelp && exit 2

    makeAnimation $offs $ampl
fi

