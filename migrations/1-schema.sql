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