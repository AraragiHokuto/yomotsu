static int __errno;

int *
__get_errno(void)
{
	return &__errno;
}
