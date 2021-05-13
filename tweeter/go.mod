module github.com/theteacat/camerathing/tweeter

go 1.15

require (
	github.com/dghubble/go-twitter v0.0.0-20201011215211-4b180d0cc78d
	github.com/dghubble/oauth1 v0.7.0
	github.com/nfnt/resize v0.0.0-20180221191011-83c6a9932646 // indirect
)

replace github.com/dghubble/go-twitter => github.com/drswork/go-twitter v0.0.0-20190721142740-110a39637298
