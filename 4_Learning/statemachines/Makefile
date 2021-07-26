# Configurable variables
SOURCE_DIR ?= bundles
BUILDDIR ?= build
DISTDIR ?= $(BUILDDIR)/dist

# File handling variables
BUNDLE_LF=$(foreach d,$(SOURCE_DIR), $(wildcard $(d)/*/sources/*.lf))
PARSED_STATEMACHINES=$(foreach f,$(BUNDLE_LF), $(BUILDDIR)/$(f:.lf=.json))

all: upload-bundles

BUNDLES=$(shell find $(SOURCE_DIR) -type f -name 'manifest.json' | sed -r 's|/[^/]+$$||' | sed -r 's|^$(SOURCE_DIR)/||' | sort | uniq)
bundles: parse $(DISTDIR)
	ride bundle compile $(BUNDLES) -B $(BUILDDIR)/bundles --ignore-dependencies -o $(DISTDIR)/$@.zip
.PHONY: bundles

upload-bundles: bundles
	ride bundle install $(DISTDIR)/bundles.zip

bundle-%: parse $(DISTDIR)
	ride bundle compile $(@:bundle-%=%) -B $(BUILDDIR)/bundles --ignore-dependencies -o $(DISTDIR)/$(patsubst bundle-%,%,$(@)).zip $(patsubst bundle-%,%,$(@))

clean:
	rm -rf $(BUILDDIR) $(DISTDIR)

parse: $(PARSED_STATEMACHINES)
	for dir in $(SOURCE_DIR); do \
		mkdir -p $(BUILDDIR)/$$dir; \
		bundles=$$(find $$dir -type f -name 'manifest.json' | sed -r 's|/[^/]+$$||' | sort | uniq); \
		for bundle in $$bundles; do \
			mkdir -p $(BUILDDIR)/$$bundle; \
			cp $$bundle/manifest.json $(BUILDDIR)/$$bundle;  \
			if [ -d $$bundle/resources ]; then cp -r $$bundle/resources $(BUILDDIR)/$$bundle; fi; \
			ln -fs sources $(BUILDDIR)/$$bundle/statemachines; \
		done; \
	done

$(BUILDDIR)/%.json: %.lf
	@mkdir -p $(shell dirname $@)
	ride cli parse -o $@ -f $<

$(DISTDIR):
	mkdir -p $(DISTDIR)
