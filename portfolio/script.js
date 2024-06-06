document.addEventListener("DOMContentLoaded", function() {
    showTab(null, "information");
});



function showTab(event, tabId) {
    event.preventDefault();
    
    // Remove active class from all tabs
    document.querySelectorAll('.tab-content').forEach(function (content) {
        content.classList.remove('active');
    });

    // Hide all tab content
    document.querySelectorAll('.tab-content').forEach(function (content) {
        content.classList.remove('d-block');
        content.classList.add('d-none');
    });

    // Remove active class from all nav links
    document.querySelectorAll('nav a').forEach(function (link) {
        link.classList.remove('active');
    });

    // Add active class to the clicked nav link
    event.currentTarget.classList.add('active');

    // Show the selected tab content
    document.getElementById(tabId).classList.remove('d-none');
    document.getElementById(tabId).classList.add('d-block');
    document.getElementById(tabId).classList.add('active');
}


function smoothScroll(id) {
    const element = document.getElementById(id);
    if (element) {
        element.scrollIntoView({
            behavior: 'smooth'
        });
    }
}


document.addEventListener('mousemove', function(e) {
    const mouseX = e.clientX;
    const mouseY = e.clientY;
    
    const cursor = document.querySelector('.cursor');
    cursor.style.top = mouseY + 'px';
    cursor.style.left = mouseX + 'px';
});


function toggleDarkMode() {
    document.body.classList.toggle('dark-mode');
    // toggle again to remove dark mode functionality
    
    if (localStorage.getItem('dark-mode') == 'true') {
        var videoSource = 'https://videos.pexels.com/video-files/854752/854752-hd_1280_720_30fps.mp4';
    } else {
        var videoSource = 'https://videos.pexels.com/video-files/854752/854752-hd_1280_720_30fps.mp4';
    }
}

