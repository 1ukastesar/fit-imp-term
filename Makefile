ARCHIVE_NAME = xtesar43.zip
DOC_BASE = README.md
DOC_BIN = README.pdf

default: compile

compile:
	idf.py build

flash:
	idf.py flash

monitor:
	idf.py monitor

doc: $(DOC_BIN)

$(DOC_BIN): $(DOC_BASE)
	pandoc $^ -o $@

clean:
	idf.py fullclean
	rm -fr sdkconfig sdkconfig.old $(DOC_BIN) $(ARCHIVE_NAME)

pack: doc
	zip -r $(ARCHIVE_NAME) main Makefile $(DOC_BASE) $(DOC_BIN) sdkconfig.defaults
