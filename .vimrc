set path+=external/**
set path+=src/**

function! RunF5()
    if g:termdebug_is_running
        execute "RunOrContinue"
    else
        execute "Termdebug bin/ray"
    endif
endfunction

nnoremap <F5> :call RunF5()<CR>

