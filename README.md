# LongView 龍閱

A unified, scrollable one-page dashboard app for managers to efficiently review configurable, multi-source reports and metrics across platforms.

## Meaning & Backstory

<p align="center">
  <strong>一屏阅尽百表事，龙阅洞察千端理</strong><br>
  <em>One screen to review a hundred reports, LongView to uncover a thousand threads.</em>
</p>
<p align="center">
  <img src="assets/banner.png" alt="LongView Banner" width="60%">
</p>

The name **LongView** carries layered meaning:

- **Long** refers both to the **Dragon (龍)** — a traditional Chinese symbol of vision, wisdom, and authority, often used to represent emperors and leaders.  Just like how emperors in ancient China would read through stacks of memorials each day to govern their empire, modern managers are tasked with reviewing countless reports and dashboards to steer their organizations.  It also reflects the **long, scrollable nature** of the app interface — a single, extended view that brings everything into one screen.

- **View** aligns with the Chinese word **“閱” (yuè)**, meaning "to read" or "to review", and shares a similar pronunciation.

Together, **LongView** reflects both the **form** and the **purpose** of the app:
a unified, scrollable interface that empowers managers to comprehensively read, review, and reason through complex reports across multiple domains — just like a modern-day dragon reviewing the affairs of their realm.

## HOWTO

### Development Environment

To set up the development environment and build the application:

1. Build the Docker development image:
```bash
docker build -t qt6-dev .
```

2. Build the application using the provided script:
```bash
./build.sh
```

The compiled application will be available at `./build/bin/LongView`.

## License

Copyright © 2025 [Jing Li](https://github.com/thyrlian)

Released under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).

See the [LICENSE file](https://github.com/thyrlian/LongView/blob/main/LICENSE) for full details.
