import sys
from pathlib import Path


def lowercase_pairs(data):
    pairs = []
    for line in data.read_text(encoding="utf-8").splitlines():
        fields = line.split(";")
        if len(fields) > 13 and fields[13]:
            point = int(fields[0], 16)
            pairs.append((point, int(fields[13], 16) - point))
    return pairs


def shifts(pairs):
    runs = []
    for point, delta in pairs:
        if runs:
            first, last, step, run_delta = runs[-1]
            single = first == last
            if run_delta == delta and ((single and point - last in (1, 2)) or (not single and point - last == step)):
                runs[-1] = (first, point, point - first if single else step, delta)
                continue
        runs.append((point, point, 1, delta))
    return runs


def main():
    here = Path(__file__).resolve().parent
    data = Path(sys.argv[1]) if len(sys.argv) > 1 else here / "UnicodeData.txt"
    target = Path(sys.argv[2]) if len(sys.argv) > 2 else here / "lower.inc"
    text = "".join("{ 0x%04X, 0x%04X, %d, %d },\n" % run for run in shifts(lowercase_pairs(data)))
    with open(target, "w", encoding="ascii", newline="\n") as out:
        out.write(text)


if __name__ == "__main__":
    main()
