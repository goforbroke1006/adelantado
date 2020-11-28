CREATE TABLE IF NOT EXISTS links
(
    id               SERIAL        NOT NULL PRIMARY KEY,
    created_at       TIMESTAMP     NOT NULL DEFAULT NOW(),
    address          VARCHAR(2048) NOT NULL UNIQUE,
    meta_title       VARCHAR(2048)          DEFAULT NULL,
    meta_description VARCHAR(2048)          DEFAULT NULL,
    body_title       VARCHAR(2048)          DEFAULT NULL,
    body_keywords    JSONB         NOT NULL DEFAULT '{}',
    checked_at       TIMESTAMP              DEFAULT NULL
);

CREATE TABLE IF NOT EXISTS search
(
    id         SERIAL,
    created_at TIMESTAMP     NOT NULL DEFAULT NOW(),
    phrase     VARCHAR(2048) NOT NULL,
    ipv4_addr  VARCHAR(24)   NOT NULL
);

ALTER TABLE links
    ADD COLUMN domain VARCHAR(1024) DEFAULT NULL;

ALTER TABLE links
    ADD COLUMN next_check_at TIMESTAMP DEFAULT NULL;

CREATE INDEX idx_links_domain ON links (domain);

ALTER TABLE links
    ADD COLUMN meta_keywords VARCHAR(2048) DEFAULT NULL;

DELETE
FROM links
WHERE address LIKE '%cgi-bin%';

-- CREATE TABLE IF NOT EXISTS domains
-- (
--     name          VARCHAR(2048) NOT NULL PRIMARY KEY,
--     next_check_at TIMESTAMP DEFAULT NULL
-- );

WITH chunk AS (
    SELECT address
    FROM links
    WHERE (address LIKE E'%\n%' OR address LIKE E'%\n')
      AND NOT EXISTS(SELECT address FROM links ll WHERE ll.address = replace(links.address, E'\n', ''))
    LIMIT 1000
)
UPDATE links
SET address = replace(links.address, E'\n', '')
FROM chunk
WHERE links.address = chunk.address;

DELETE
FROM links
WHERE links.address IN (
    SELECT address
    FROM links
    WHERE (address LIKE E'%\n%' OR address LIKE E'%\n')
      AND EXISTS(SELECT address FROM links ll WHERE ll.address = replace(links.address, E'\n', ''))
    LIMIT 1000
);


ALTER TABLE links
    ADD COLUMN og_title       VARCHAR(2048) DEFAULT NULL,
    ADD COLUMN og_image       VARCHAR(2048) DEFAULT NULL,
    ADD COLUMN og_description VARCHAR(2048) DEFAULT NULL,
    ADD COLUMN og_site_name   VARCHAR(2048) DEFAULT NULL;

ALTER TABLE links
    ADD COLUMN last_status_code  SMALLINT    DEFAULT NULL,
    ADD COLUMN page_content_size INT         DEFAULT NULL,
    ADD COLUMN charset           VARCHAR(64) DEFAULT NULL;
