for f in flatbufferschemas/*.fbs; do
  flatc --cpp --scoped-enums -o gen/g2/gfx "$f"
done