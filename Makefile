release-msvc: clean SunProjectInteractive-msvc-source.zip SunProjectInteractive-win.zip

SunProjectInteractive-msvc-source.zip:
	zip -r SunProjectInteractive-msvc-source.zip * -x \*.zip \*/.\* Makefile

SunProjectInteractive-win.zip:
	( \
		cd msvc/Release || exit 1; \
		zip -r ../../SunProjectInteractive-win.zip * -x \*/.\* \*.obj help/images/bitmaps/\* \
	)

clean:
	rm -f SunProjectInteractive-msvc-source.zip SunProjectInteractive-win.zip
