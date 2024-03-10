#!/usr/bin/env sh

# no new line at the end of text
printf "first normal line\nno new line at the end" | ./flbng

# ascii art
figlet flash bang test | ./flbng

# random fortune text
/usr/games/fortune | ./flbng

# as much lines as terminal height
lines() {
    for i in $(seq $(tput lines))
        do echo "$i"
    done
}
lines | ./flbng

# as much chars as terinal widht
chars () {
    for i in $(seq $(tput cols))
        do printf "*"
    done
}
chars | ./flbng

./flbng <<EOF
Some errors check. Should be:
    flbng: too much lines for this terminal, should be less than X
    flbng: too long line for this terminal, should be shorter than X
EOF

echo "0\n$(lines)" | ./flbng
echo "*$(chars)" | ./flbng
