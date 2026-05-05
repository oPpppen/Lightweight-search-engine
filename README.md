# Search Engine Coursework

## Dependencies

- `muduo`
- `cppjieba`
- `utfcpp`
- `simhash`
- `tinyxml2`
- `nlohmann_json`
- C++17 compiler

The current code assumes `cppjieba`, `utfcpp`, and `simhash` headers are available under `/usr/local/include`, `cppjieba` dictionaries are available under `/usr/local/dict`, and `tinyxml2` plus `muduo` are installed on the system.

## Build

From the repository root:

```bash
mkdir -p build
cd build
cmake ..
make -j
```

After building, the main executables are generated in the top-level `build/` directory:

- `keyword_builder`
- `webpage_builder`
- `search_server`
- `search_client`
- `test_keyword`
- `test_websearch`

## Generate Offline Data

Still in `build/`:

```bash
./keyword_builder
./webpage_builder
```

This generates:

- `build/data/ch_dict.dat`
- `build/data/ch_index.dat`
- `build/data/en_dict.dat`
- `build/data/en_index.dat`
- `build/data/pages.dat`
- `build/data/offsets.dat`
- `build/data/inverted_index.dat`

## Prepare Online Data

Return to the repository root and run:

```bash
./SearchEngineOnline/scripts/prepare_online_data.sh
```

This copies the offline output into:

- `SearchEngineOnline/data/`
- `SearchEngineOnline/build/data/` if that directory exists
- top-level `build/data/`

The online service uses:

- `data/ch_dict.dat`
- `data/ch_index.dat`
- `data/en_dict.dat`
- `data/en_index.dat`
- `data/pages.dat`
- `data/offsets.dat`
- `data/inverted_index.dat`
- `data/stopwords_cn.txt`

## Start The Server

From `build/`:

```bash
./search_server
```

Or choose a custom port:

```bash
./search_server 9999
```

## Start The Client

From another terminal in `build/`:

```bash
./search_client 127.0.0.1 8888
```

Interactive commands:

- `type 1`: keyword recommendation
- `type 2`: webpage search
- `type 0`: exit

The client also accepts other numeric `type` values, which is useful for testing server-side error handling.

## Request Meanings

### `type 1`

Keyword recommendation.

- Input: a Chinese or English query string
- Output: top 5 candidate words in JSON

Example response:

```json
{
  "type": "keyword",
  "query": "中国",
  "result": ["中国", "中", "中心", "我国", "美国"]
}
```

### `type 2`

Webpage search.

- Input: a Chinese query string
- Processing: cppjieba segmentation, stopword filtering, TF-IDF query vector, candidate intersection, cosine similarity ranking
- Output: top 10 search results in JSON

Example response:

```json
{
  "type": "search",
  "query": "中国",
  "result": [
    {
      "id": 387,
      "title": "中国特色社会主义制度体现优越性（国际论坛）",
      "link": "http://cpc.people.com.cn/n1/2021/0517/c64387-32104960.html",
      "abstract": "中国共产党是有能力、有担当的执政党..."
    }
  ]
}
```

## TLV Protocol

This project keeps the original TLV framing used by `Codec`.

Packet layout:

```text
+---------+----------------+-------------------+
| 1 byte  |    4 bytes     |    N bytes        |
+---------+----------------+-------------------+
| type    | length (uint32)| value payload     |
+---------+----------------+-------------------+
```

Details:

- `type`: request type, stored as `uint8_t`
- `length`: payload length in network byte order
- `value`: UTF-8 request body

Current meanings:

- `1`: keyword recommendation
- `2`: webpage search
- other values: server returns an error JSON

## Suggested End-To-End Run Order

```bash
mkdir -p build
cd build
cmake ..
make -j
./keyword_builder
./webpage_builder
cd ..
./SearchEngineOnline/scripts/prepare_online_data.sh
cd build
./search_server
```

Then in another terminal:

```bash
cd /path/to/search_engine/build
./search_client 127.0.0.1 8888
```
