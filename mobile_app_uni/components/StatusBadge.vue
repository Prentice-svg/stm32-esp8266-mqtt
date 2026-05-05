<template>
	<text class="status-badge" :class="type">{{ text }}</text>
</template>

<script>
export default {
	name: 'StatusBadge',
	props: {
		text: {
			type: String,
			default: '正常'
		},
		type: {
			type: String,
			default: 'success'
		}
	}
}
</script>

<style>
.status-badge {
	min-width: 72rpx;
	padding: 9rpx 18rpx;
	border-radius: 999rpx;
	font-size: 22rpx;
	line-height: 1.2;
	font-weight: 850;
	text-align: center;
	border: 1rpx solid currentColor;
}

.status-badge.success {
	color: #16a34a;
	background: rgba(34, 197, 94, 0.12);
}

.status-badge.warning {
	color: #d97706;
	background: rgba(245, 158, 11, 0.14);
}

.status-badge.danger {
	color: #dc2626;
	background: rgba(239, 68, 68, 0.12);
}

.status-badge.muted {
	color: #64748b;
	background: rgba(148, 163, 184, 0.16);
}
</style>
