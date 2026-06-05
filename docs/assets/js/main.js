/**
 * CuffnCode - Dokumentasi GitHub Pages
 * JavaScript: theme toggle, sidebar, lightbox, smooth scroll
 */

document.addEventListener('DOMContentLoaded', function() {

    /* ======================== Theme Toggle ======================== */
    const themeToggle = document.getElementById('theme-toggle');
    const storedTheme = localStorage.getItem('cuffncode-theme') || 'light';

    function setTheme(theme) {
        document.documentElement.setAttribute('data-theme', theme);
        localStorage.setItem('cuffncode-theme', theme);
        if (themeToggle) {
            themeToggle.innerHTML = theme === 'dark'
                ? '<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="5"/><line x1="12" y1="1" x2="12" y2="3"/><line x1="12" y1="21" x2="12" y2="23"/><line x1="4.22" y1="4.22" x2="5.64" y2="5.64"/><line x1="18.36" y1="18.36" x2="19.78" y2="19.78"/><line x1="1" y1="12" x2="3" y2="12"/><line x1="21" y1="12" x2="23" y2="12"/><line x1="4.22" y1="19.78" x2="5.64" y2="18.36"/><line x1="18.36" y1="5.64" x2="19.78" y2="4.22"/></svg>'
                : '<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"/></svg>';
        }
    }

    setTheme(storedTheme);

    if (themeToggle) {
        themeToggle.addEventListener('click', function() {
            const current = document.documentElement.getAttribute('data-theme');
            setTheme(current === 'dark' ? 'light' : 'dark');
        });
    }

    /* ======================== Mobile Sidebar ======================== */
    const menuToggle = document.getElementById('menu-toggle');
    const sidebar = document.getElementById('sidebar');

    if (menuToggle && sidebar) {
        menuToggle.addEventListener('click', function() {
            sidebar.classList.toggle('open');
        });

        /* Tutup sidebar saat klik di luar */
        document.addEventListener('click', function(e) {
            if (window.innerWidth <= 768) {
                if (!sidebar.contains(e.target) && !menuToggle.contains(e.target)) {
                    sidebar.classList.remove('open');
                }
            }
        });
    }

    /* ======================== Active Link Highlighting ======================== */
    const sidebarLinks = document.querySelectorAll('.sidebar-nav a');

    function updateActiveLink() {
        const scrollPos = window.scrollY + 120;
        let currentId = '';

        document.querySelectorAll('h1[id], h2[id], h3[id], h4[id]').forEach(section => {
            const offsetTop = section.offsetTop;
            if (scrollPos >= offsetTop) {
                currentId = section.getAttribute('id');
            }
        });

        sidebarLinks.forEach(link => {
            link.classList.remove('active');
            if (link.getAttribute('href') === '#' + currentId) {
                link.classList.add('active');
            }
        });
    }

    window.addEventListener('scroll', updateActiveLink);

    /* ======================== Smooth Scroll ======================== */
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function(e) {
            const href = this.getAttribute('href');
            if (href === '#') return;

            const target = document.querySelector(href);
            if (target) {
                e.preventDefault();
                const offsetTop = target.offsetTop - 80;
                window.scrollTo({
                    top: offsetTop,
                    behavior: 'smooth'
                });

                /* Tutup sidebar mobile */
                if (sidebar && window.innerWidth <= 768) {
                    sidebar.classList.remove('open');
                }
            }
        });
    });

    /* ======================== Image Lightbox ======================== */
    const lightbox = document.createElement('div');
    lightbox.className = 'lightbox-overlay';
    lightbox.innerHTML = '<img src="" alt="Enlarged image">';
    document.body.appendChild(lightbox);

    const lightboxImg = lightbox.querySelector('img');

    document.querySelectorAll('.image-container img, .main-content img:not(.no-lightbox)').forEach(img => {
        img.style.cursor = 'pointer';
        img.addEventListener('click', function() {
            lightboxImg.src = this.src;
            lightbox.classList.add('active');
            document.body.style.overflow = 'hidden';
        });
    });

    lightbox.addEventListener('click', function() {
        this.classList.remove('active');
        document.body.style.overflow = '';
    });

    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape' && lightbox.classList.contains('active')) {
            lightbox.classList.remove('active');
            document.body.style.overflow = '';
        }
    });

    /* ======================== Copy Code Button ======================== */
    document.querySelectorAll('pre').forEach(pre => {
        const button = document.createElement('button');
        button.className = 'copy-btn';
        button.textContent = 'Copy';

        button.addEventListener('click', function() {
            const code = pre.querySelector('code');
            const text = code ? code.textContent : pre.textContent;

            navigator.clipboard.writeText(text).then(() => {
                button.textContent = 'Copied!';
                setTimeout(() => {
                    button.textContent = 'Copy';
                }, 2000);
            }).catch(() => {
                /* Fallback for older browsers */
                const textarea = document.createElement('textarea');
                textarea.value = text;
                document.body.appendChild(textarea);
                textarea.select();
                document.execCommand('copy');
                document.body.removeChild(textarea);
                button.textContent = 'Copied!';
                setTimeout(() => {
                    button.textContent = 'Copy';
                }, 2000);
            });
        });

        pre.appendChild(button);
    });

    /* ======================== Lazy Loading ======================== */
    if ('loading' in HTMLImageElement.prototype) {
        document.querySelectorAll('.main-content img:not(.no-lazy)').forEach(img => {
            img.loading = 'lazy';
        });
    }

    /* ======================== Sidebar Nav Generation ======================== */
    /* Auto-generate sidebar from headings if using template */
    const sidebarNav = document.querySelector('.sidebar-nav');
    if (sidebarNav && sidebarNav.querySelectorAll('a').length === 0) {
        const headings = document.querySelectorAll('.main-content h2, .main-content h3');

        headings.forEach(heading => {
            if (!heading.id) {
                heading.id = heading.textContent.toLowerCase()
                    .replace(/[^\w\s]/g, '')
                    .replace(/\s+/g, '-');
            }

            const li = document.createElement('li');
            li.className = heading.tagName === 'H3' ? 'nav-sub' : '';

            const a = document.createElement('a');
            a.href = '#' + heading.id;
            a.textContent = heading.textContent;
            li.appendChild(a);
            sidebarNav.appendChild(li);
        });
    }

    console.log('CuffnCode docs loaded. Theme:', storedTheme);
});
